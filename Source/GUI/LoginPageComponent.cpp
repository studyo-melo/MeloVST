//
// Created by Padoa on 08/01/2025.
//
#include "LoginPageComponent.h"
#include "../API/MeloApiService.h"

LoginPageComponent::LoginPageComponent() {
    addAndMakeVisible(title);
    title.setText("Bienvenue sur la Page de Login", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    // Créer le champ de texte pour le nom d'utilisateur
    addAndMakeVisible(usernameLabel);
    usernameLabel.setText("Nom d'utilisateur:", juce::dontSendNotification);

    addAndMakeVisible(usernameField);
    usernameField.setTextToShowWhenEmpty("Entrez votre nom", juce::Colours::grey);

    // Créer le champ de texte pour le mot de passe
    addAndMakeVisible(passwordLabel);
    passwordLabel.setText("Mot de passe:", juce::dontSendNotification);

    addAndMakeVisible(passwordField);
    passwordField.setPasswordCharacter('*');  // Masquer les caractères du mot de passe

    // Créer le bouton de connexion
    addAndMakeVisible(loginButton);
    loginButton.setButtonText("Se connecter");
    loginButton.onClick = [this] { onLoginButtonClick(); };
}

LoginPageComponent::~LoginPageComponent() = default;

void LoginPageComponent::resized() {
    auto area = getLocalBounds().reduced(20);  // Ajouter un peu de padding
    int fieldHeight = 40;

    usernameLabel.setBounds(area.removeFromTop(fieldHeight));
    usernameField.setBounds(area.removeFromTop(fieldHeight));

    passwordLabel.setBounds(area.removeFromTop(fieldHeight));
    passwordField.setBounds(area.removeFromTop(fieldHeight));

    loginButton.setBounds(area.removeFromTop(fieldHeight));
}

void LoginPageComponent::onLoginButtonClick() const {
    juce::String email = usernameField.getText();
    juce::String password = passwordField.getText();

    juce::StringPairArray jsonBody;
    jsonBody.set("email", email);
    jsonBody.set("password", password);

    MeloApiService::getInstance().makePOSTRequest(ApiRoute::PostLogin, jsonBody);

    // Logique de validation (par exemple, vérifier les informations)
    // if (username == "admin" && password == "password") {
    //     juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
    //         "Connexion réussie", "Bienvenue, " + username);
    // } else {
    //     juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
    //         "Erreur de connexion", "Nom d'utilisateur ou mot de passe incorrect.");
    // }
}

void LoginPageComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::lightblue);
}