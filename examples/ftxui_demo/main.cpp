#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>

int main()
{
    auto doc = ftxui::hbox({ ftxui::text("FTXUI demo OK") | ftxui::bold | ftxui::color(ftxui::Color::Green) });
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(doc));
    ftxui::Render(screen, doc);
    std::cout << screen.ToString() << std::endl;
    return 0;
}
