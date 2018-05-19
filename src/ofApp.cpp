#include "ofApp.h"
#include "simon.h"

std::vector<ofFile> shuffle_file_list(std::string path);
float delay{1.}; // time in second between each note
float init_delay{1.}; // initial delay

enum GameStatus { WAIT_INPUT, // wait for input
                  PLAY_TONE, // play melody
                  PLAY_MOVIE, // play movie sequence
                  LOSE, WIN,
                  GAME_OVER,
                  TIME_OUT,
                  WAIT_PLAYER, // wait for a player
                  START_SPLASH, // player number and level announcement
                  CREDITS // display credit video
                };
GameStatus status;

void tone(int c);
void light(int c);
bool new_tone{true};
void reset();

std::vector<ofSoundPlayer> samplers;
std::vector<Player> players;
int current_player{};
// level 0 : 8 steps
// level 1 : 10 steps
// level 2 : 14 steps
// level 3 : until death, speed increase
int level{0};
int sequence_size{14};
ofFbo fbo;
ofSerial serial;

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

void setup_serial()
{
  vector <ofSerialDeviceInfo> list = serial.getDeviceList();
  ofLogNotice("Serial") << "Device list:";
  for (ofSerialDeviceInfo& d : list)
  {
    ofLogNotice("Serial") << "name: " << d.getDeviceName() << " id " << d.getDeviceID() << " path " << d.getDevicePath();
  }

#ifdef __APPLE__
  const char* dev = "/dev/cu.usbmodem1D1311";
#else
  const char* dev = "/dev/ttyACM0";
#endif
  serial.setup(dev,9600);
  serial.flush();
  serial.writeByte(RED_OFF);
  serial.writeByte(GREEN_OFF);
  serial.writeByte(YELLOW_OFF);
  serial.writeByte(BLUE_OFF);
  serial.writeByte(TONE_OFF);
}

void ofApp::setup()
{
  ofSetLogLevel(OF_LOG_NOTICE);
  ofSetLogLevel("Simon Leone", OF_LOG_NOTICE);

  // attende
  m_wait = shuffle_file_list("movie/attente/");
  m_credits = shuffle_file_list("movie/credits/");
  m_splash = shuffle_file_list("movie/splash/");

  std::vector<std::string> sounds = {
      "sound/son1.mp3",
      "sound/son2.mp3",
      "sound/son3.mp3",
      "sound/son4.mp3",
      "sound/gagne.mp3",
      "sound/loupe.mp3"
  };

  samplers.resize(6);
  int i = 0;
  for (const auto& file : sounds)
  {
      try {
        samplers[i].load(file);
      } catch (...) {
          ofLogError("Simon Leone") << "oups sorry, je trouve pas le son " << file;
      }
      i++;
  }

  for (auto& sampler : samplers)
  {
    if(sampler.isLoaded())
    {
      sampler.setVolume(0.75f);
      sampler.setMultiPlay(false);
    }
  }

  ofSetFrameRate(25);
  ofSetBackgroundColor(ofColor::black);
  fbo.allocate(ofGetWidth(), ofGetHeight());

  m_font.load("verdana.ttf", 14, true, true);
  m_font.setLineHeight(18.0f);
  m_font.setLetterSpacing(1.037);

  setup_serial();
  reset();
}

std::vector<ofFile> shuffle_file_list(std::string path)
{
  ofDirectory dir;

  // dir.allowExt("mp4");
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
    case CREDITS:
      credits();
      break;
    default:
      ofLogError("Simon Leone") << "Bad state !";
      ;
  }
}

void ofApp::start_splash(){
  ofLogVerbose("Simon Leone") << "Start Splash !";

  if(m_splash.empty())
  {
    m_player.close();
    status = PLAY_TONE;
    current_player=0;
    ofResetElapsedTimeCounter();
    return;
  }

  if (!m_player.isLoaded())
  {
    int index = ofRandom(m_splash.size());
    m_player.load(m_splash[index].path());
    m_player.setLoopState(OF_LOOP_NONE);
    m_player.play();
  }
  else if (m_player.getIsMovieDone())
  {
    m_player.close();
    status = PLAY_TONE;
    current_player=0;
    ofResetElapsedTimeCounter();
  }
}

void Player::play_melody()
{
  auto time = ofGetElapsedTimef();

  ofLogVerbose("Simon Leone") << "Play Melody ! " << time;

  if( time < delay)
  {
    if (m_seq_it == m_sequence.end())
    {
      ofLogVerbose("Simon Leaone") << "Wait for input";
      m_seq_it = m_sequence.begin();
      tone(-1); // switch off LED & TONE
      status = WAIT_INPUT;
      ofResetElapsedTimeCounter();
    }
    else
    {
      tone(m_seq_it->first);
      light(m_seq_it->first);
      if ( !serial.isInitialized()
           && m_seq_it == m_sequence.end() - 1
           && !samplers[m_seq_it->first].isPlaying())
      {
        m_seq_it++;
      }
    }
  } else {
    m_seq_it++;
    new_tone=true;
    ofResetElapsedTimeCounter();
  }
}

void ofApp::wait_player()
{
  ofLogVerbose("Simon Leone") << "Wait player";
  if(serial.isInitialized())
  {
    while(serial.available())
    {
      char b = serial.readByte();
      level=-1;
      switch(b)
      {
        case BTN_LEVEL1:
          level=1;
          break;
        case BTN_LEVEL2:
          level=2;
          break;
        case BTN_LEVEL3:
          level=3;
          break;
        case BTN_LEVEL4:
          level=4;
          break;
        default:
          continue;
      }
      start_new_game();
      return;
    }
  }
  if (!m_player.isLoaded() || m_player.getIsMovieDone())
  {
    auto file = m_wait[ofRandom(m_wait.size())].path();
    ofLogVerbose("Simon Leone") << "load new waiting file: " << file;
    m_player.load(file);
    m_player.setLoopState(OF_LOOP_NONE);
    m_player.play();
  }
}

void ofApp::wait_input()
{
  if(ofGetElapsedTimef() > delay )
    status = TIME_OUT;
  ofLogNotice("Simon Leone") << "wait for " << ofGetElapsedTimef() << " ms" ;

  if(serial.isInitialized())
  {
    while(serial.available()
          && status == WAIT_INPUT)
    {
      char b = serial.readByte();
      switch(b)
      {
        case BTN_RED:
          record_key(0);
          break;
        case BTN_GREEN:
          record_key(2);
          break;
        case BTN_YELLOW:
          record_key(1);
          break;
        case BTN_BLUE:
          record_key(3);
          break;
        default:
          ;
      }
    }
  }
}

void ofApp::credits()
{
  ofLogVerbose("Simon Leone") << "Credits !";

  if(m_credits.empty())
  {
    m_player.close();
    reset();
    return;
  }

  if (!m_player.isLoaded())
  {
    int index = ofRandom(m_credits.size());
    m_player.load(m_credits[index].path());
    m_player.setLoopState(OF_LOOP_NONE);
    m_player.play();
  }
  else if (m_player.getIsMovieDone())
  {
    m_player.close();
    reset();
  }
}

void ofApp::timeout()
{
  ofLogVerbose("Simon Leone") << "Timeout !";
  new_tone = true;
  tone(5);
  while(samplers[5].isPlaying())
  {;}
  status = LOSE;
}

void Player::lose()
{
  ofLogVerbose("Simon Leone") << "Lose !";
  if (!m_player.isLoaded())
  {
    std::vector<std::vector<ofFile>* > colors =
    { &m_perdu_rouge, &m_perdu_jaune, &m_perdu_vert, &m_perdu_bleu };
    auto choice = colors[m_seq_it->first];
    int index = ofRandom(choice->size());
    m_player.load((*choice)[index].path());
    m_player.setLoopState(OF_LOOP_NONE);
    m_player.play();
  }
  else if (m_player.getIsMovieDone())
  {
    m_player.close();
    status=CREDITS;
  }
}

void Player::win()
{
  ofLogVerbose("Simon Leone") << "Win !";
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
  ofLogVerbose("Simon Leone") << "Replay ! " << current_player << " "
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
        ofLogNotice("Simon Leone") << "choose another sequence: " << m_sequence.back().second
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

      if (*m_answer_it == color)
      {
        ofClear(colors[color]);

        m_player.load(m_seq_it->second);
        m_player.setLoopState(OF_LOOP_NONE);
        m_player.play();

        m_seq_it++;
        m_answer_it++;
      }
      else
      {
        m_player.close();
        status = LOSE;
        new_tone = true;
      }
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
  if(!serial.isInitialized())
  {
    ofPushStyle();
    ofSetColor(ofColor(255,255,255,128));
    ofRectangle bbox = m_font.getStringBoundingBox(text,0,0);
    ofPoint anchor = center + ofPoint(-bbox.getWidth()/2,bbox.height);
    bbox.x = anchor.x - 5.;
    bbox.width += 10.;
    bbox.y = anchor.y-bbox.height-5.;
    bbox.height += 10.;
    // ofDrawRectangle(bbox);
    ofSetColor(ofColor::white);
    m_font.drawString(text, anchor.x, anchor.y);
    ofPopStyle();
  }
}

void ofApp::draw()
{
  ofLogVerbose("Simon Leone") << "current player: " << current_player;

  if(status == PLAY_TONE)
  {
    if (!serial.isInitialized())
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
      draw_text("Vert : trop facile", ofPoint(ofGetWidth()/2, 50));
      draw_text("Bleu : t'es à l'aise", ofPoint(ofGetWidth()/2, 80));
      draw_text("Jaune : fait gaffe à toi", ofPoint(ofGetWidth()/2, 110));
      draw_text("Rouge : t'as chaud au cul", ofPoint(ofGetWidth()/2, 140));
    }
  }

  if(status == START_SPLASH)
  {
    // ofClear(ofColor::black);
    std::ostringstream oss;
    oss << "Start with " << players.size() << " player";
    if (players.size() > 1)
       oss << "s";
    oss << "\n" << "Level: " << level;
    draw_text(oss.str());
  }

  if(status == PLAY_TONE)
  {
    std::ostringstream oss;
    oss << "Player " << current_player+1;
    draw_text(oss.str());
  }
}

void ofApp::start_new_game()
{
  players.clear();
  current_player=1;
  ofLogNotice("Simon Leone") << "start new game with level " << level;
  switch(level)
  {
    case 1:
      sequence_size=8;
      delay=2.;
      break;
    case 2:
      sequence_size=11;
      delay=1.5;
      break;
    case 3:
      sequence_size=14;
      delay=1.;
      break;
    case 4:
      sequence_size=1000;
      delay=0.99;
      init_delay=delay;
      break;
    default:
      return;
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
}

void ofApp::keyPressed(ofKeyEventArgs& key)
{
  ofLogNotice("Simon Leone") << "key pressed: " << key.key;

  switch (status) {
    case WAIT_INPUT:
      {
        switch(key.key)
        {
          case OF_KEY_LEFT: // Green
            record_key(2);
            break;
          case OF_KEY_RIGHT: // Bleu
            record_key(3);
            break;
          case OF_KEY_UP: // Red
            record_key(0);
            break;
          case OF_KEY_DOWN: // Yellow
            record_key(1);
            break;
          default:
            return;
        }
        break;
      }
    case WAIT_PLAYER:
      {
        level=-1;
        switch(key.key)
        {
          case OF_KEY_LEFT:
            level=1;
            break;
          case OF_KEY_RIGHT:
            level=2;
            break;
          case OF_KEY_DOWN:
            level=3;
            break;
          case OF_KEY_UP:
            level=4;
            break;
          default:
            return;
            ;
        }
        start_new_game();
        break;
      }
    default:
      ;
  }
}

void ofApp::record_key(int key)
{
  players[current_player].m_answer.push_back(key);
  players[current_player].m_answer_it = players[current_player].m_answer.end() - 1;


  new_tone = true;
  int sample = key;
  if(*players[current_player].m_answer_it != players[current_player].m_seq_it->first)
  {
    players[current_player].m_seq_it = players[current_player].m_sequence.end();
    sample = 5;
  } else {
    players[current_player].m_seq_it++;
  }

  if(!serial.isInitialized())
    tone(sample);

  ofResetElapsedTimeCounter();

  if (players[current_player].m_seq_it == players[current_player].m_sequence.end())
  {
    if(!serial.isInitialized())
    {
      while(samplers[sample].isPlaying())
      {}
    }
    status = PLAY_MOVIE;
    if (level==3)
      delay *= init_delay;
    players[current_player].m_seq_it = players[current_player].m_sequence.begin();
    players[current_player].m_answer_it = players[current_player].m_answer.begin();
  }

  if(serial.available())
    serial.flush();
}

void tone(int c)
{
  // 0 RED, 1 GREEN, 2 YELLOW, 3 BLUE
  if(serial.isInitialized())
  {
    switch(c)
    {
      case 0:
        serial.writeByte(RED_ON);
        serial.writeByte(RED_TONE);
        break;
      case 1:
        serial.writeByte(GREEN_ON);
        serial.writeByte(GREEN_TONE);
        break;
      case 2:
        serial.writeByte(YELLOW_ON);
        serial.writeByte(YELLOW_TONE);
        break;
      case 3:
        serial.writeByte(BLUE_ON);
        serial.writeByte(BLUE_TONE);
        break;
      case 4:
        serial.writeByte(RED_OFF);
        serial.writeByte(GREEN_OFF);
        serial.writeByte(YELLOW_OFF);
        serial.writeByte(BLUE_OFF);
        serial.writeByte(WIN_TONE);
        break;
      case 5:
        serial.writeByte(RED_OFF);
        serial.writeByte(GREEN_OFF);
        serial.writeByte(YELLOW_OFF);
        serial.writeByte(BLUE_OFF);
        serial.writeByte(LOSE_TONE);
        break;
      default:
        serial.writeByte(RED_OFF);
        serial.writeByte(GREEN_OFF);
        serial.writeByte(YELLOW_OFF);
        serial.writeByte(BLUE_OFF);
        serial.writeByte(TONE_OFF);
    }
  }
  else
  {
    if(new_tone)
    {
      for(auto& sampler : samplers)
          sampler.stop();
      samplers[c].play();
      new_tone=false;
    }
  }
}

void light(int c)
{
  // NOTE: c < 0 turn off the lights
  // 0 RED, 1 GREEN, 2 YELLOW, 3 BLUE

  if(serial.isInitialized())
  {

  } else {

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
}

void reset()
{
  players.clear();

  status = WAIT_PLAYER;
  ofResetElapsedTimeCounter();
  new_tone = true;
}
