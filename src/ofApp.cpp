#include "ofApp.h"

void ofApp::setup()
{
  ofSetLogLevel(OF_LOG_NOTICE);

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

  // attende
  m_attente = shuffle_file_list("movie/attente/");

  m_samplers.resize(6);
  m_samplers[0].load("sound/son1.mp3");
  m_samplers[1].load("sound/son2.mp3");
  m_samplers[2].load("sound/son3.mp3");
  m_samplers[3].load("sound/son4.mp3");
  m_samplers[4].load("sound/gagne.mp3");
  m_samplers[5].load("sound/loupe.mp3");

  for (auto& sampler : m_samplers)
  {
    sampler.setVolume(0.75f);
    sampler.setMultiPlay(false);
  }

  ofSetFrameRate(25);
  ofSetBackgroundColor(ofColor::black);
  m_fbo.allocate(ofGetWidth(), ofGetHeight());

  reset();
}

std::vector<ofFile> ofApp::shuffle_file_list(std::string path)
{
  ofDirectory dir;

  dir.allowExt("mp4");
  dir.listDir(path);
  vector<ofFile> shuffled = dir.getFiles();
  std::random_shuffle(begin(shuffled),end(shuffled));

  return std::move(shuffled);
}

std::pair<int, std::string> ofApp::random_choice()
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

void ofApp::update()
{
  switch(m_status)
  {
    case WAIT_INPUT:
      wait_input();
      break;
    case PLAY_TONE:
      play_melody();
      break;
    case PLAY_MOVIE:
      play_movie_sequence();
      break;
    case TIME_OUT:
      timeout();
      break;
    case GAME_OVER:
      credits();
      break;
    case LOSE:
      lose();
      break;
    case WIN:
      win();
      break;
    case WAIT_PLAYER:
      wait_player();
      break;
    default:
      ofLogError("Simon Leone") << "Bad state !";
      ;
  }
}

void ofApp::play_melody()
{
  auto time = ofGetElapsedTimef();

  ofLogNotice("Simon Leon") << "Play ! " << time;

  if( time < m_delay)
  {
    if (m_seq_it == m_sequence.end())
    {
      m_seq_it = m_sequence.begin();
      m_status = WAIT_INPUT;
    }
    else
    {
      tone(m_seq_it->first);
      light(m_seq_it->first);
    }
  } else {
    m_seq_it++;
     m_new_tone=true;
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

  if(ofGetElapsedTimef() > (m_sequence_size * m_delay) )
    m_status = TIME_OUT;
}

void ofApp::credits()
{
  ofLogNotice("Simon Leon") << "Credits !";
}

void ofApp::timeout()
{
  ofLogNotice("Simon Leon") << "Timeout !";
}

void ofApp::lose()
{
  ofLogNotice("Simon Leon") << "Lose !";
  tone(4);
  while(m_samplers[4].isPlaying())
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

void ofApp::win()
{
  ofLogNotice("Simon Leon") << "Win !";
  tone(5);
  while(m_samplers[5].isPlaying())
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

void ofApp::play_movie_sequence()
{
  ofLogNotice("Simon Leon") << "Replay ! "
                            << m_seq_it->first << " " <<  m_sequence.size() << " "
                            << *m_answer_it << " " << m_answer.size();

  if (!m_player.isLoaded() || m_player.getIsMovieDone())
  {
    // si on a déjà joué toutes les réponses
    if ( m_seq_it == m_sequence.end() )
    {
      // et qu'on a joué assez de séquences
      if(m_sequence.size() == m_sequence_size)
      {
        // alors on a gagné
        m_player.close();
        m_status = WIN;
        m_new_tone = true;
      } else {
        // sinon on ajoute une nouvelle séquence et on recommence
        m_player.close();
        m_sequence.push_back(random_choice());
        ofLogNotice("Simon Leon") << "choose another sequence: " << m_sequence.back().second
                                  << " size: " << m_sequence.size();

        m_status = PLAY_TONE;
        m_seq_it = m_sequence.begin();
        m_answer.clear();
        m_answer_it = m_answer.begin();
        m_new_tone=true;
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

void ofApp::draw()
{

  if(m_status == PLAY_TONE)
  {
    m_fbo.draw(0.,0.);
  }
  else if(m_player.isPlaying())
  {
    float w, h;
    float window_ratio = ofGetWidth() / ofGetHeight();
    float player_ratio = m_player.getWidth() / m_player.getHeight();
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
    m_player.update();
    m_player.draw(0,0,w,h);
  }
}

void ofApp::keyPressed(ofKeyEventArgs& key)
{
  ofLogNotice("Simon Leon") << "key pressed: " << key.key;

  switch (m_status) {
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
      m_status = PLAY_TONE;
      m_player.close();
      ofResetElapsedTimeCounter();
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
  m_answer.push_back(key);
  m_answer_it = --m_answer.end();

  if(key == m_seq_it->first)
  {
    ofResetElapsedTimeCounter();
    m_new_tone = true;
    tone(key);
    m_seq_it++;

    if (m_seq_it == m_sequence.end())
    {
      m_status = PLAY_MOVIE;
      m_seq_it = m_sequence.begin();
      m_answer_it = m_answer.begin();
    }
  }
  else
  {
    m_status = LOSE;
    m_new_tone = true;
  }
}

void ofApp::tone(int c)
{
  if(m_new_tone)
  {
    m_samplers[c].play();
    m_new_tone=false;
  }
}

void ofApp::light(int c)
{
  // NOTE: c < 0 turn off the lights

  m_fbo.begin();
  {
    ofClear(ofColor::black);

    ofColor color;
    ofLogVerbose("Simon Leone") << "light " << c;

    if(!m_samplers[c].isPlaying())
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
  m_fbo.end();
}

void ofApp::reset()
{
  m_sequence.clear();
  m_sequence.reserve(m_sequence_size);
  m_sequence.push_back(random_choice());
  m_seq_it = m_sequence.begin();

  m_answer.clear();
  m_answer_it = m_answer.begin();
  m_status = WAIT_PLAYER;
  ofResetElapsedTimeCounter();
  m_new_tone = true;
}
