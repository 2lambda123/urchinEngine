#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include <3d/scene/ui/widget/textbox/TextBoxTest.h>
#include <AssertHelper.h>
using namespace urchin;

void TextBoxTest::textShift() {
    auto uiRenderer = setupUiRenderer();
    auto textBox = TextBox::create(nullptr, Position(0.0f, 0.0f, PIXEL), Size(50.0f, 20.0f, PIXEL), "test");
    uiRenderer->addWidget(textBox);

    std::string textValue = "abcdefg"; //text box can only display 'abcdef'
    uiRenderer->onMouseMove(1.0f, 1.0f); //move mouse over text box
    uiRenderer->onKeyPress((int)InputDeviceKey::MOUSE_LEFT); //activate text box
    for (char textLetter : textValue) {
        uiRenderer->onChar(static_cast<char32_t>(textLetter));
    }
    AssertHelper::assertStringEquals(textBox->getTextWidget().getBaseText(), textValue.substr(TextFieldConst::LETTER_SHIFT));

    for (std::size_t i = 0; i < textValue.size(); ++i) {
        uiRenderer->onKeyPress((int)InputDeviceKey::LEFT_ARROW);
    }
    AssertHelper::assertStringEquals(textBox->getTextWidget().getBaseText(), "abcdef");
}

std::unique_ptr<UIRenderer> TextBoxTest::setupUiRenderer() {
    renderTarget = std::make_unique<NullRenderTarget>(1920, 1080);
    i18nService = std::make_unique<I18nService>();
    UISkinService::instance().setSkin("ui/skinDefinition.uda");

    return std::make_unique<UIRenderer>(*renderTarget, *i18nService);
}

CppUnit::Test* TextBoxTest::suite() {
    auto* suite = new CppUnit::TestSuite("TextBoxTest");

    suite->addTest(new CppUnit::TestCaller<TextBoxTest>("textShift", &TextBoxTest::textShift));

    return suite;
}