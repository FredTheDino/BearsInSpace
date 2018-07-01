#pragma once

#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_PRESSED 2
#define KEY_RELEASED 3

#include <unordered_map>
#include <string>

namespace SK
{
  class Key
  {
  public:
    
    static void update()
    {
      for (auto ks : _state_map)
      {
	if (ks.second == KEY_PRESSED)
	  _state_map[ks.first] = KEY_DOWN;
	else if (ks.second == KEY_RELEASED)
	  _state_map[ks.first] = KEY_UP;
      }
    }

    static void reg(std::string name, int key)
    {
      _name_map.emplace(key, name);
      _state_map.emplace(name, KEY_UP);
    }

    static bool up(std::string name) { return _state_map[name] == KEY_UP || released(name); }
    
    static bool down(std::string name) { return _state_map[name] == KEY_DOWN || pressed(name); }
    
    static bool pressed(std::string name) { return _state_map[name] == KEY_PRESSED; }
    
    static bool released(std::string name) { return _state_map[name] == KEY_RELEASED; }

    static void setKeyState(int key, int state)
    {
      if (_name_map.find(key) != _name_map.end())
	_state_map[_name_map.at(key)] = state;
    }
    
  private:
    static std::unordered_map<int, std::string> _name_map;
    static std::unordered_map<std::string, int> _state_map;
  };
}
