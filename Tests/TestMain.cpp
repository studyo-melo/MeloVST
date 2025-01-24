#include <juce_core/juce_core.h>

int main()
{
    juce::UnitTestRunner runner;
    runner.setPassesAreLogged(true); // Affiche les tests réussis.
    runner.runAllTests();

    return 0;
}