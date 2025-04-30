#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <chrono>
#include <thread>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <fstream>
#include <random>
#include <filesystem>

using namespace ftxui;


std::mutex file_mutex;
const std::string shared_filename = "timer_shared_file.txt";


std::string generateRandomContent(int timer_id) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distrib(0, 25);
    
    std::stringstream ss;
    ss << "Timer #" << timer_id << " writing at " << std::chrono::system_clock::now().time_since_epoch().count() 
       << " - Random data: ";
    
    
    for(int i = 0; i < 20; i++) {
        ss << static_cast<char>('a' + distrib(gen));
    }
    ss << std::endl;
    
    return ss.str();
}

struct Timer {
    int id;
    int seconds;
    std::string name;
    std::string startTime;
    std::string endTime;
    std::atomic<bool> isRunning{false};
    std::atomic<bool> isPaused{false};
    std::atomic<bool> isCompleted{false};
    std::atomic<bool> isWaitingForFile{false};
    std::chrono::time_point<std::chrono::system_clock> endTimePoint;
    std::chrono::seconds remainingTime{0};
    std::string status;
};

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    return ss.str();
}

int main() {
    std::string input_seconds;
    std::string input_name = "Timer";
    std::string status_message;
    std::mutex message_mutex;
    std::vector<std::shared_ptr<Timer>> timers;
    std::mutex timers_mutex;
    int next_timer_id = 1;
    
    auto screen = ScreenInteractive::TerminalOutput();

    auto seconds_input = Input(&input_seconds, "Enter seconds");
    auto name_input = Input(&input_name, "Timer name");
    
    auto start_button = Button("Start New Timer", [&] {
        if (!input_seconds.empty()) {
            try {
                int seconds = std::stoi(input_seconds);
                if (seconds > 0) {
                    auto now = std::chrono::system_clock::now();
                    auto timer = std::make_shared<Timer>();
                    timer->id = next_timer_id++;
                    timer->seconds = seconds;
                    timer->name = input_name.empty() ? "Timer " + std::to_string(timer->id) : input_name;
                    timer->startTime = getCurrentTime();
                    timer->isRunning = true;
                    timer->endTimePoint = now + std::chrono::seconds(seconds);
                    timer->remainingTime = std::chrono::seconds(seconds);

                    
                    {
                        std::lock_guard<std::mutex> lock(timers_mutex);
                        timers.push_back(timer);
                    }
                    
                    
                    input_seconds = "";
                    
                    
                    std::thread([timer, &screen, &timers_mutex]() {
                        auto start_time = std::chrono::system_clock::now();
                        auto end_time = start_time + std::chrono::seconds(timer->seconds);
                        bool has_written_to_file = false;
                        
                        while (std::chrono::system_clock::now() < end_time && !timer->isCompleted) {
                            
                            auto now = std::chrono::system_clock::now();
                            auto remaining = std::chrono::duration_cast<std::chrono::seconds>(end_time - now).count();
                            timer->remainingTime = std::chrono::seconds(remaining);
                            
                            
                            if (remaining <= 10 && !has_written_to_file && !timer->isPaused) {
                                timer->isWaitingForFile = true;
                                timer->status = "Waiting for file access...";
                                screen.PostEvent(Event::Custom);
                                
                                
                                if (file_mutex.try_lock()) {
                                    
                                    timer->isWaitingForFile = false;
                                    timer->status = "Writing to file...";
                                    screen.PostEvent(Event::Custom);
                                    
                                    try {
                                        
                                        std::ofstream file(shared_filename, std::ios::app);
                                        
                                        if (file.is_open()) {
                                            auto write_start = std::chrono::system_clock::now();
                                            
                                            
                                            while (std::chrono::system_clock::now() < end_time && !timer->isCompleted) {
                                                file << generateRandomContent(timer->id);
                                                file.flush();
                                                
                                                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                                                
                                                
                                                now = std::chrono::system_clock::now();
                                                remaining = std::chrono::duration_cast<std::chrono::seconds>(
                                                    end_time - now).count();
                                                timer->remainingTime = std::chrono::seconds(remaining);
                                            }
                                            
                                            file.close();
                                            has_written_to_file = true;
                                        }
                                    } catch (...) {
                                        timer->status = "Error writing to file!";
                                    }
                                    
                                    
                                    file_mutex.unlock();
                                    timer->status = "";
                                    screen.PostEvent(Event::Custom);
                                } else {
                                    
                                    timer->isPaused = true;
                                    timer->status = "Paused - waiting for file access";
                                    
                                    auto pause_start = std::chrono::system_clock::now();
                                    
                                    
                                    std::unique_lock<std::mutex> lock(file_mutex);
                                    timer->isWaitingForFile = false;
                                    timer->isPaused = false;
                                    
                                    
                                    auto pause_duration = std::chrono::system_clock::now() - pause_start;
                                    end_time += pause_duration;
                                    
                                    timer->status = "Writing to file after waiting...";
                                    screen.PostEvent(Event::Custom);
                                    
                                    try {
                                        
                                        std::ofstream file(shared_filename, std::ios::app);
                                        
                                        if (file.is_open()) {
                                            
                                            while (std::chrono::system_clock::now() < end_time && !timer->isCompleted) {
                                                file << generateRandomContent(timer->id);
                                                file.flush();
                                                
                                                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                                                
                                                
                                                now = std::chrono::system_clock::now();
                                                remaining = std::chrono::duration_cast<std::chrono::seconds>(
                                                    end_time - now).count();
                                                timer->remainingTime = std::chrono::seconds(remaining);
                                            }
                                            
                                            file.close();
                                            has_written_to_file = true;
                                        }
                                    } catch (...) {
                                        timer->status = "Error writing to file!";
                                    }
                                    
                                    
                                    lock.unlock();
                                    timer->status = "";
                                    screen.PostEvent(Event::Custom);
                                }
                            }
                            
                            
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                        
                        timer->endTime = getCurrentTime();
                        timer->isRunning = false;
                        timer->isCompleted = true;
                        timer->status = "Completed";
                        
                        
                        screen.PostEvent(Event::Custom);
                    }).detach();
                    
                    
                    {
                        std::lock_guard<std::mutex> lock(message_mutex);
                        status_message = "Started timer: " + timer->name + " for " + 
                            std::to_string(seconds) + " second(s)";
                    }
                    
                    
                    screen.PostEvent(Event::Custom);
                }
            } catch (...) {
                std::lock_guard<std::mutex> lock(message_mutex);
                status_message = "Invalid input! Please enter a valid number for seconds.";
            }
        }
    });

    auto clear_button = Button("Clear Completed", [&] {
        bool any_cleared = false;
        {
            std::lock_guard<std::mutex> lock(timers_mutex);
            auto it = timers.begin();
            while (it != timers.end()) {
                if ((*it)->isCompleted) {
                    it = timers.erase(it);
                    any_cleared = true;
                } else {
                    ++it;
                }
            }
        }
        
        if (any_cleared) {
            std::lock_guard<std::mutex> lock(message_mutex);
            status_message = "Cleared completed timers";
        }
        
        screen.PostEvent(Event::Custom);
    });

    auto delete_file_button = Button("Delete Shared File", [&] {
        std::lock_guard<std::mutex> lock(file_mutex);
        bool deleted = false;
        
        try {
            if (std::filesystem::exists(shared_filename)) {
                std::filesystem::remove(shared_filename);
                deleted = true;
            }
        } catch (...) {}
        
        {
            std::lock_guard<std::mutex> msg_lock(message_mutex);
            status_message = deleted ? 
                "Shared file deleted successfully" : 
                "File does not exist or could not be deleted";
        }
        
        screen.PostEvent(Event::Custom);
    });

    auto controls = Container::Vertical({
        Container::Horizontal({
            seconds_input,
            name_input
        }),
        Container::Horizontal({
            start_button,
            clear_button,
            delete_file_button
        })
    });

    auto renderer = Renderer(controls, [&] {
        
        std::vector<Element> timer_elements;
        {
            std::lock_guard<std::mutex> lock(timers_mutex);
            for (const auto& timer : timers) {
                std::string status;
                Color status_color = Color::White;
                
                if (timer->isCompleted) {
                    status = "✓ Completed at " + timer->endTime;
                    status_color = Color::Green;
                } else if (timer->isPaused) {
                    status = "⏸ Paused - Waiting for file access";
                    status_color = Color::Red;
                } else if (timer->isWaitingForFile) {
                    status = "⌛ Waiting for file access";
                    status_color = Color::Magenta;
                } else if (timer->isRunning) {
                    std::string remaining_str = std::to_string(timer->remainingTime.count());
                    status = "⏱ Running: " + remaining_str + "s remaining";
                    
                    if (timer->remainingTime.count() <= 10) {
                        if (!timer->status.empty()) {
                            status += " - " + timer->status;
                        }
                        status_color = Color::Yellow;
                    } else {
                        status_color = Color::Blue;
                    }
                }
                
                timer_elements.push_back(
                    hbox({
                        text(timer->name) | flex | bold,
                        text(" | "),
                        text(status) | color(status_color)
                    }) | border
                );
            }
        }
        
        
        if (timer_elements.empty()) {
            timer_elements.push_back(text("No active timers") | center);
        }

        
        std::string file_status;
        try {
            if (std::filesystem::exists(shared_filename)) {
                file_status = "File exists, size: " + 
                    std::to_string(std::filesystem::file_size(shared_filename)) + " bytes";
            } else {
                file_status = "Shared file does not exist yet";
            }
        } catch (...) {
            file_status = "Error checking file status";
        }

        
        std::string display_message;
        {
            std::lock_guard<std::mutex> lock(message_mutex);
            display_message = status_message;
        }

        return vbox({
            text("Multi-Timer Application with File Sharing") | bold | center,
            separator(),
            hbox({
                vbox({
                    text("Seconds:") | bold,
                    seconds_input->Render()
                }),
                vbox({
                    text("Name:") | bold,
                    name_input->Render()
                })
            }),
            hbox({
                start_button->Render(),
                clear_button->Render(),
                delete_file_button->Render()
            }),
            separator(),
            text(display_message) | center | color(Color::CadetBlue),
            text(file_status) | center | color(Color::GrayDark),
            text("File: " + shared_filename) | center,
            separator(),
            text("Active Timers:") | bold | center,
            vbox(timer_elements) | flex,
            text("Note: Timers will try to access shared file when 10 seconds remain") | center | color(Color::GrayDark)
        }) | border;
    });

    
    std::atomic<bool> refresh_running = true;
    std::thread([&]() {
        while (refresh_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            screen.PostEvent(Event::Custom);
        }
    }).detach();

    screen.Loop(renderer);
    
    
    refresh_running = false;
    
    return 0;
}