#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxSerial.h"#include "ofxSerial.h"


class Player {
  public:
    Player(const int size);

    void play_melody();
    void play_movie_sequence();
    void lose();
    void win();

    enum SimonColor {
      RED = 0,
      GREEN,
      YELLOW,
      BLUE
    };

    std::vector<std::pair<Player::SimonColor,std::string>> m_sequence{};
    std::vector<std::pair<Player::SimonColor,std::string>>::iterator m_seq_it{};
    std::vector<Player::SimonColor> m_answer{};
    std::vector<Player::SimonColor>::iterator m_answer_it{};

    std::vector<ofFile> m_vert;
    std::vector<ofFile> m_jaune;
    std::vector<ofFile> m_rouge;
    std::vector<ofFile> m_bleu;

    std::vector<ofFile> m_perdu_vert;
    std::vector<ofFile> m_perdu_jaune;
    std::vector<ofFile> m_perdu_rouge;
    std::vector<ofFile> m_perdu_bleu;

    std::vector<ofFile> m_gagne_vert;
    std::vector<ofFile> m_gagne_jaune;
    std::vector<ofFile> m_gagne_rouge;
    std::vector<ofFile> m_gagne_bleu;

    std::pair<Player::SimonColor,std::string> random_choice();

    ofVideoPlayer m_playerA, m_playerB;
    ofVideoPlayer* m_player;

private:
    ofVideoPlayer* swap_player();
};

class ofApp : public ofBaseApp
{
  public:
    void setup  ();
    void update ();
    void draw   ();

    void keyPressed(ofKeyEventArgs&);

    void wait_input(); // wait for user input
    void wait_player(); // wait for choosing a player number
    void record_key(Player::SimonColor key);
    void draw_text(const std::string& text, ofPoint center=ofPoint(ofGetWidth()/2,440));

    void credits();
    void timeout();
    void start_splash();
    void start_new_game();

    void reset();

    std::vector<ofFile> m_wait;
    std::vector<ofFile> m_credits;
    std::vector<ofFile> m_splash;
#include "ofxSerial.h"

    ofVideoPlayer m_playerA, m_playerB;
    ofVideoPlayer* m_player{&m_playerA};

    ofVideoPlayer* swap_player();

private:
    struct Level {
      int steps; // total number of steps
      float delay; // delay in ms between each step
      float factor{1.}; // delay is multiply by factor each time a new step is added
    };

    void setup_serial();
    void reset_serial();
    void load_settings();

    ofTrueTypeFont m_font;
    ofxXmlSettings m_xml;
    std::string m_serial_device_string;
    Level m_levels[4];
};
