#include "ofApp.h"

void ofApp::setup()
{
  ofSetLogLevel(OF_LOG_VERBOSE);

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

  dir.listDir(path);
  vector<ofFile> shuffled = dir.getFiles();
  std::random_shuffle(begin(shuffled),end(shuffled));

  return std::move(shuffled);
}

std::pair<int, std::string> ofApp::random_choice()
{
  std::vector<std::vector<ofFile>* > colors =
    { &m_rouge, &m_jaune, &m_vert, &m_bleu };

  int color = ofRandom(3);
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
}

void ofApp::draw()
{
  switch(m_status)
  {
    case WAIT:
      wait();
      break;
    case PLAY:
      play_sequence();
      break;
    case REPLAY:
      replay_sequence();
      break;
    case TIME_OUT:
      timeout();
      break;
    case GAME_OVER:
      credits();
      break;
    case LOOSE:
      loose();
      break;
    case WIN:
      win();
      break;
    default:
      ;
  }
}

void ofApp::play_sequence()
{
  auto time = ofGetElapsedTimef();

  ofLogNotice("Simon Leon") << "Play ! " << time;

  if( time < m_delay)
  {
    if (m_seq_it == m_sequence.end())
    {
      m_seq_it = m_sequence.begin();
      m_status = WAIT;
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
  m_fbo.draw(0,0);
}

void ofApp::wait()
{
  // ofLogNotice("Simon Leone") << "wait ";

  if(ofGetElapsedTimef() > (14.0 * m_delay) )
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

void ofApp::loose()
{
  ofLogNotice("Simon Leon") << "Loose !";
  tone(4);
}

void ofApp::win()
{
  ofLogNotice("Simon Leon") << "Win !";
  tone(5);
}

void ofApp::replay_sequence()
{
  ofLogNotice("Simon Leon") << "Replay !";

  m_player.update();
  if (!m_player.isLoaded() || m_player.getIsMovieDone())
  {
    // si on a déjà joué toutes les réponses
    if ( m_seq_it == m_sequence.end() )
    {
      // mais qu'on n'a pas atteint 14
      if(m_sequence.size() != 14)
      {
        // alors on ajoute une nouvelle séquence et on recommence
        m_sequence.push_back(random_choice());
        ofLogNotice("Simon Leon") << "choose another sequence: " << m_sequence.back().second
                                  << " size: " << m_sequence.size();

        m_status = PLAY;
        m_seq_it = m_sequence.begin();
        m_new_tone=true;
        ofResetElapsedTimeCounter();
      } else {
        // sinon on a gagné
        m_status = WIN;
        m_new_tone = true;
      }
    } else {

      std::vector<ofColor> colors = {
        ofColor::red,
        ofColor::yellow,
        ofColor::green,
        ofColor::blue };

      int color = m_seq_it->first;

      ofClear(colors[color]);

      if(m_seq_it->first == *m_answer_it)
      {
        m_player.load(m_seq_it->second);
        m_player.setLoopState(OF_LOOP_NONE);
        m_player.play();
      }
      else
      {
        std::vector<std::vector<ofFile>* > colors =
        { &m_perdu_rouge, &m_perdu_jaune, &m_perdu_vert, &m_perdu_bleu };
        auto choice = colors[*m_answer_it];
        int index = ofRandom(choice->size()-1);
        m_player.load((*choice)[index].path());
        m_player.setLoopState(OF_LOOP_NONE);
        m_player.play();
      }

      m_seq_it++;
      m_answer_it++;
    }
  }
  else
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
    m_player.draw(0,0,w,h);
  }
}

void ofApp::keyPressed(ofKeyEventArgs& key)
{
  ofLogNotice("Simon Leon") << "key pressed: " << key.key;

  switch (m_status) {
    case WAIT:
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
    case REPLAY:
    case PLAY:
      break;
    default:
      reset();
      ;
  }
}

void ofApp::record_key(int key)
{
  m_answer.push_back(key);

  if(key == m_seq_it->first)
  {
    ofResetElapsedTimeCounter();
    m_new_tone = true;
    tone(key);
    m_seq_it++;

    if (m_seq_it == m_sequence.end())
    {
      m_status = REPLAY;
      m_seq_it = m_sequence.begin();
      m_answer_it = m_answer.begin();
    }
  }
  else
  {
    m_status = LOOSE;
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
  m_sequence.reserve(14);
  m_sequence.push_back(random_choice());
  m_seq_it = m_sequence.begin();

  m_answer.clear();
  m_answer_it = m_answer.begin();
  m_status = PLAY;
  ofResetElapsedTimeCounter();
  m_new_tone = true;
}
