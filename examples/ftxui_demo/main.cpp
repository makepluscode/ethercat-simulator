#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>

int main()
{
    using namespace ftxui;
    auto doc = hbox({ text("FTXUI demo OK") | bold | color(Color::Green) });
    auto screen = Screen::Create(Dimension::Fit(doc));
    Render(screen, doc);
    std::cout << screen.ToString() << std::endl;
    return 0;
}

