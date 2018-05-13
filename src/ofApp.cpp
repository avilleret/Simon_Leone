#include "ofApp.h"



std::vector<ofFile> shuffle_file_list(std::string path);
float delay{1.}; // time in second between each note

enum GameStatus { WAIT_INPUT, // wait for input
                  PLAY_TONE, // play melody
                  PLAY_MOVIE, // play movie sequence
                  LOSE, WIN,
                  GAME_OVER,
                  TIME_OUT,
                  WAIT_PLAYER, // wait for a player
                  START_SPLASH // player number announcement
                };
GameStatus status;

void tone(int c);
void light(int c);
bool new_tone{true};
void reset();

std::vector<ofSoundPlayer> samplers;
std::vector<Player> players;
int current_player{};
int sequence_size{14};
ofFbo fbo;

Player::Player(const int size)
{
  m_rouge = shuffle_file_list("movie/rouge/");
  m_jaune = shuffle_file_list("movie/jaune/");
  m_vert  = shuffle_file_list("movie/vert/");
  m_bleu  = shuffle_file_list("movie/bleu/");

  // perdu
  m_perdu_rouge = shuffle_file_list("movie/perdu-rouge/");
  m_perdu_jaune = shuffle_file_list("movie/perdu-jaune/");
  m_perdu_vert  = shuffle_file_list("movie/perdu-vert/");
  m_perdu_bleu  = shuffle_file_list("movie/perdu-bleu/");

  // gagne
  m_gagne_rouge = shuffle_file_list("movie/gagne-rouge/");
  m_gagne_jaune = shuffle_file_list("movie/gagne-jaune/");
  m_gagne_vert  = shuffle_file_list("movie/gagne-vert/");
  m_gagne_bleu  = shuffle_file_list("movie/gagne-bleu/");

  m_sequence.clear();
  m_sequence.reserve(size);
  m_sequence.push_back(random_choice());
  m_seq_it = m_sequence.begin();
  m_answer.clear();
  m_answer_it = m_answer.begin();
}

std::pair<int, std::string> Player::random_choice()
{
  std::vector<std::vector<ofFile>* > colors =
    { &m_rouge, &m_jaune, &m_vert, &m_bleu };

  int color = ofRandom(4);
  auto shuffled = colors[color];

  auto file = shuffled->back();
  shuffled->pop_back();

  while(!m_player.load(file.path()))
  {
    file = shuffled->back();
    shuffled->pop_back();
  }
  m_player.close();
  return {color, file.path()};
}

void ofApp::setup()
{
  ofSetLogLevel(OF_LOG_NOTICE);

  // attende
  m_attente = shuffle_file_list("movie/attente/");

  samplers.resize(6);
  samplers[0].load("sound/son1.mp3");
  samplers[1].load("sound/son2.mp3");
  samplers[2].load("sound/son3.mp3");
  samplers[3].load("sound/son4.mp3");
  samplers[4].load("sound/gagne.mp3");
  samplers[5].load("sound/loupe.mp3");

  for (auto& sampler : samplers)
  {
    sampler.setVolume(0.75f);
    sampler.setMultiPlay(false);
  }

  ofSetFrameRate(25);
  ofSetBackgroundColor(ofColor::black);
  fbo.allocate(ofGetWidth(), ofGetHeight());

  m_font.load("verdana.ttf", 14, true, true);
  m_font.setLineHeight(18.0f);
  m_font.setLetterSpacing(1.037);

  reset();
}

std::vector<ofFile> shuffle_file_list(std::string path)
{
  ofDirectory dir;

  dir.allowExt("mp4");
  dir.listDir(path);
  vector<ofFile> shuffled = dir.getFiles();
  std::random_shuffle(begin(shuffled),end(shuffled));

  return std::move(shuffled);
}

void ofApp::update()
{
  switch(status)
  {
    case WAIT_INPUT:
      wait_input();
      break;
    case PLAY_TONE:
      players[current_player].play_melody();
      break;
    case PLAY_MOVIE:
      players[current_player].play_movie_sequence();
      break;
    case TIME_OUT:
      timeout();
      break;
    case GAME_OVER:
      credits();
      break;
    case LOSE:
      players[current_player].lose();
      break;
    case WIN:
      players[current_player].win();
      break;
    case WAIT_PLAYER:
      wait_player();
      break;
    case START_SPLASH:
      start_splash();
      break;
    default:
      ofLogError("Simon Leone") << "Bad state !";
      ;
  }
}

void ofApp::start_splash(){
  if(ofGetElapsedTimef() > 3.)
  {
    status = PLAY_TONE;
    current_player=0;
    ofResetElapsedTimeCounter();
  }
}

void Player::play_melody()
{
  auto time = ofGetElapsedTimef();

  ofLogNotice("Simon Leon") << "Play ! " << time;

  if( time < delay)
  {
    if (m_seq_it == m_sequence.end())
    {
      m_seq_it = m_sequence.begin();
      status = WAIT_INPUT;
    }
    else
    {
      tone(m_seq_it->first);
      light(m_seq_it->first);
    }
  } else {
    m_seq_it++;
    new_tone=true;
    ofResetElapsedTimeCounter();
  }
}

void ofApp::wait_player()
{
  if (!m_player.isLoaded() || m_player.getIsMovieDone())
  {
    auto file = m_attente[ofRandom(m_attente.size())].path();
    ofLogNotice("Simon Leone") << "load new waiting file: " << file;
    m_player.load(file);
    m_player.setLoopState(OF_LOOP_NONE);
    m_player.play();
  }
}

void ofApp::wait_input()
{
  // ofLogNotice("Simon Leone") << "wait ";

  if(ofGetElapsedTimef() > (sequence_size * delay) )
    status = TIME_OUT;
}

void ofApp::credits()
{
  ofLogNotice("Simon Leon") << "Credits !";
}

void ofApp::timeout()
{
  ofLogNotice("Simon Leon") << "Timeout !";
  status = LOSE;
}

void Player::lose()
{
  ofLogNotice("Simon Leon") << "Lose !";
  tone(4);
  while(samplers[4].isPlaying())
  {
    return;
  }
  if (!m_player.isLoaded())
  {
    std::vector<std::vector<ofFile>* > colors =
    { &m_perdu_rouge, &m_perdu_jaune, &m_perdu_vert, &m_perdu_bleu };
    auto choice = colors[*m_answer_it];
    int index = ofRandom(choice->size());
    m_player.load((*choice)[index].path());
    m_player.setLoopState(OF_LOOP_NONE);
    m_player.play();
  } else if (m_player.getIsMovieDone())
  {
    m_player.close();
    reset();
  }
}

void Player::win()
{
  ofLogNotice("Simon Leon") << "Win !";
  tone(5);
  while(samplers[5].isPlaying())
  {
    return;
  }
  if (!m_player.isLoaded())
  {
    std::vector<std::vector<ofFile>* > colors =
    { &m_gagne_rouge, &m_gagne_jaune, &m_gagne_vert, &m_gagne_bleu };
    auto choice = colors[m_sequence.back().first];
    int index = ofRandom(choice->size());
    m_player.load((*choice)[index].path());
    m_player.setLoopState(OF_LOOP_NONE);
    m_player.play();
  } else if (m_player.getIsMovieDone())
  {
    m_player.close();
    reset();
  }
}

void Player::play_movie_sequence()
{
  ofLogNotice("Simon Leon") << "Replay ! " << current_player << " "
                            << m_seq_it->first << " " <<  m_sequence.size() << " "
                            << *m_answer_it << " " << m_answer.size();

  if (!m_player.isLoaded() || m_player.getIsMovieDone())
  {
    // si on a déjà joué toutes les réponses
    if ( m_seq_it == m_sequence.end() )
    {
      // et qu'on a joué assez de séquences
      if(m_sequence.size() == sequence_size)
      {
        // alors on a gagné
        m_player.close();
        status = WIN;
        new_tone = true;
      } else {
        // sinon on ajoute une nouvelle séquence et on recommence
        m_player.close();
        m_sequence.push_back(random_choice());
        ofLogNotice("Simon Leon") << "choose another sequence: " << m_sequence.back().second
                                  << " size: " << m_sequence.size();

        current_player++;
        current_player%=players.size();
        status = PLAY_TONE;
        m_seq_it = m_sequence.begin();
        m_answer.clear();
        m_answer_it = m_answer.begin();
        new_tone=true;
        ofResetElapsedTimeCounter();
      }
    } else {

      std::vector<ofColor> colors = {
        ofColor::red,
        ofColor::yellow,
        ofColor::green,
        ofColor::blue };

      int color = m_seq_it->first;

      ofClear(colors[color]);

      m_player.load(m_seq_it->second);
      m_player.setLoopState(OF_LOOP_NONE);
      m_player.play();

      m_seq_it++;
      m_answer_it++;
    }
  }
}

void draw_video(ofVideoPlayer& p)
{
  float w, h;
  float window_ratio = ofGetWidth() / ofGetHeight();
  float player_ratio = p.getWidth() / p.getHeight();
  if ( window_ratio > player_ratio)
  {
    h = ofGetHeight();
    w = h * player_ratio;
  }
  else
  {
    w = ofGetWidth();
    h = w / player_ratio;
  }
  ofClear(ofColor::black);
  p.update();
  p.draw(0,0,w,h);
}

void ofApp::draw_text(const std::string& text, ofPoint center)
{
  ofPushStyle();
  ofSetColor(ofColor(255,255,255,128));
  ofRectangle bbox = m_font.getStringBoundingBox(text,0,0);
  ofPoint anchor = center + ofPoint(-bbox.getWidth()/2,bbox.height);
  bbox.x = anchor.x - 5.;
  bbox.width += 10.;
  bbox.y = anchor.y-bbox.height-5.;
  bbox.height += 10.;
  ofDrawRectangle(bbox);
  ofSetColor(ofColor::red);
  m_font.drawString(text, anchor.x, anchor.y);
  ofPopStyle();
}

void ofApp::draw()
{
  ofLogNotice("Simon Leone") << "current player: " << current_player;

  if(status == PLAY_TONE)
  {
    fbo.draw(0.,0.);
  }
  else if(m_player.isPlaying())
  {
    draw_video(m_player);
  } else if(!players.empty() && players[current_player].m_player.isPlaying())
    draw_video(players[current_player].m_player);

  if (status == WAIT_PLAYER)
  {
    if (int(ofGetElapsedTimef()*2) % 2 == 0)
    {
      draw_text("Press a button to start");
      draw_text("Green : 1 player", ofPoint(ofGetWidth()/2, 50));
      draw_text("Blue : 2 players", ofPoint(ofGetWidth()/2, 80));
      draw_text("Red : 3 players", ofPoint(ofGetWidth()/2, 110));
      draw_text("Yellow : 4 players", ofPoint(ofGetWidth()/2, 140));
    }
  }

  if(status == START_SPLASH)
  {
    ofClear(ofColor::black);
    std::ostringstream oss;
    oss << "Start with " << players.size() << " player";
    if (players.size() > 1)
       oss << "s";
    draw_text(oss.str());
  }

  if(status == PLAY_TONE)
  {
    std::ostringstream oss;
    oss << "Player " << current_player+1;
    draw_text(oss.str());
  }
}

void ofApp::keyPressed(ofKeyEventArgs& key)
{
  ofLogNotice("Simon Leon") << "key pressed: " << key.key;

  switch (status) {
    case WAIT_INPUT:
    {
      switch(key.key)
      {
        case OF_KEY_LEFT:
          record_key(2);
          break;
        case OF_KEY_RIGHT:
          record_key(3);
          break;
        case OF_KEY_UP:
          record_key(0);
          break;
        case OF_KEY_DOWN:
          record_key(1);
          break;
        default:
          ;
      }
      break;
    }
    case WAIT_PLAYER:
      players.clear();
      current_player=0;
      switch(key.key)
      {
        case OF_KEY_LEFT:
          current_player=1;
          break;
        case OF_KEY_RIGHT:
          current_player=2;
          break;
        case OF_KEY_UP:
          current_player=3;
          break;
        case OF_KEY_DOWN:
          current_player=4;
          break;
        default:
          ;
      }
      ofLogNotice("Simon Leone") << "current player: " << current_player;
      while(current_player>0)
      {
        players.emplace_back(sequence_size);
        --current_player;
      }
      ofLogNotice("Simon Leone") << "player number: " << players.size();
      if(players.size()>0)
      {
        status = START_SPLASH;
        m_player.close();
        ofResetElapsedTimeCounter();
        current_player = 0;
      }
      break;
    case PLAY_MOVIE:
    case PLAY_TONE:
    case LOSE:
    case WIN:
      break;
    default:
      reset();
      ;
  }
}

void ofApp::record_key(int key)
{
  players[current_player].m_answer.push_back(key);
  players[current_player].m_answer_it = players[current_player].m_answer.end() - 1;

  if(key == players[current_player].m_seq_it->first)
  {
    ofResetElapsedTimeCounter();
    new_tone = true;
    tone(key);
    players[current_player].m_seq_it++;

    if (players[current_player].m_seq_it == players[current_player].m_sequence.end())
    {
      status = PLAY_MOVIE;
      players[current_player].m_seq_it = players[current_player].m_sequence.begin();
      players[current_player].m_answer_it = players[current_player].m_answer.begin();
    }
  }
  else
  {
    status = LOSE;
    new_tone = true;
  }
}

void tone(int c)
{
  if(new_tone)
  {
    samplers[c].play();
    new_tone=false;
  }
}

void light(int c)
{
  // NOTE: c < 0 turn off the lights

  fbo.begin();
  {
    ofClear(ofColor::black);

    ofColor color;
    ofLogVerbose("Simon Leone") << "light " << c;

    if(!samplers[c].isPlaying())
      c=-1;

    color = ofColor::red;
    color.a = c==0 ? 255 : 128;
    ofSetColor(color);
    ofDrawCircle(0.5*ofGetWidth(),ofGetHeight()*0.10,100.);

    color = ofColor::yellow;
    color.a = c==1 ? 255 : 128;
    ofSetColor(color);
    ofDrawCircle(0.5*ofGetWidth(),ofGetHeight()*0.6,100.);

    color = ofColor::green;
    color.a = c==2 ? 255 : 128;
    ofSetColor(color);
    ofDrawCircle(0.25*ofGetWidth(),ofGetHeight()*0.35,100.);

    color = ofColor::blue;
    color.a = c==3 ? 255 : 128;
    ofSetColor(color);
    ofDrawCircle(0.75*ofGetWidth(),ofGetHeight()*0.35,100.);
  }
  fbo.end();
}

void reset()
{
  players.clear();

  status = WAIT_PLAYER;
  ofResetElapsedTimeCounter();
  new_tone = true;
}
