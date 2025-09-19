#pragma once

#include <atomic>
#include <chrono>
#include <csignal>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace ethercat_sim::app {

namespace detail {

inline std::atomic_bool*& signalStopFlag() noexcept {
    static std::atomic_bool* flag = nullptr;
    return flag;
}

inline void signalHandler(int) noexcept {
    if (auto* flag = signalStopFlag()) {
        flag->store(true);
    }
}

} // namespace detail

inline void installSignalHandlers(std::atomic_bool& stop_flag) noexcept {
    detail::signalStopFlag() = &stop_flag;

    struct sigaction sa {};
    sa.sa_handler = detail::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGTSTP, &sa, nullptr);
    std::signal(SIGPIPE, SIG_IGN);
}

class TerminalGuard {
  public:
    TerminalGuard() noexcept {
        fd_     = STDIN_FILENO;
        is_tty_ = (::isatty(fd_) != 0);
        if (!is_tty_) {
            return;
        }
        if (tcgetattr(fd_, &oldt_) != 0) {
            return;
        }
        struct termios raw = oldt_;
        cfmakeraw(&raw);
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(fd_, TCSANOW, &raw) == 0) {
            active_ = true;
        }
    }

    ~TerminalGuard() {
        if (active_) {
            tcsetattr(fd_, TCSANOW, &oldt_);
        }
    }

    TerminalGuard(TerminalGuard const&)            = delete;
    TerminalGuard& operator=(TerminalGuard const&) = delete;
    TerminalGuard(TerminalGuard&&)                 = delete;
    TerminalGuard& operator=(TerminalGuard&&)      = delete;

    bool isActive() const noexcept {
        return active_;
    }
    bool isTty() const noexcept {
        return is_tty_;
    }

    bool pollForEscape(std::chrono::milliseconds timeout) const noexcept {
        if (!active_) {
            return false;
        }
        struct pollfd pfd {
            fd_, POLLIN, 0
        };
        int wait_ms = static_cast<int>(timeout.count());
        int pr      = ::poll(&pfd, 1, wait_ms);
        if (pr > 0 && (pfd.revents & POLLIN)) {
            unsigned char ch = 0;
            ssize_t       n  = ::read(fd_, &ch, 1);
            if (n == 1 && ch == 27) {
                return true;
            }
        }
        return false;
    }

  private:
    int            fd_{-1};
    bool           is_tty_{false};
    bool           active_{false};
    struct termios oldt_ {};
};

} // namespace ethercat_sim::app
