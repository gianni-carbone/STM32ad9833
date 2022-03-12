// ******************************************************************************** / 
// check a string is positive integer number
// ******************************************************************************** / 
bool isUint(String str){
   for(byte i=0;i<str.length();i++)
      if(!isDigit(str.charAt(i))) return false;
   return true;;
}

// ******************************************************************************** / 
// check a string is a float number
// ******************************************************************************** / 
bool isFloat(String str){
   bool sign = false;
   bool dot = false; 
   
   for(byte i=0;i<str.length();i++){
      if(!isDigit(str.charAt(i))) {
        if (str.charAt(i) == '.') {
          if (dot) return false; else dot = true;   
        } else if (str.charAt(i) == '-') {
          if (sign) return false; else sign = true;   
        } else return false;
      }
   }
   return true;;
}

// ******************************************************************************** / 
// get the part n of a string using given delimiter (default = space) 
// ex. part 0 of "cmd first" is "cmd", part 1 is "first"
// ******************************************************************************** / 
String str_pull(String s, uint8_t part, char dlm = ' '){
  int p = 0;
  int e = 0;
  
  for (uint8_t i = 0; i<part; i++) {
    p = s.indexOf(dlm, p+1);
    if (p < 0) return String("");
  }
  e = s.indexOf(dlm, p+1);
  if (part) p++;
  return (e<0)?s.substring(p):s.substring(p, e);
}
// ******************************************************************************** / 
// get the part n in the form of uint8_t. return false if the part is not an uint8_t
// ******************************************************************************** / 
bool str_pull(String s, uint8_t pos, uint8_t* ret, char dlm = ' '){
  int32_t r;
  String p = str_pull(s, pos, dlm);
  if (p==String("")) return false;
  if (!isUint(p)) return false;
  r = p.toInt();
  if (r<0) return false;
  if (r>255) return false;
  *ret = r;
  return true;
}

// ******************************************************************************** / 
// get the part n in the form of uint16_t. return false if the part is not an uint16_t
// ******************************************************************************** / 
bool str_pull(String s, uint8_t pos, uint16_t* ret, char dlm = ' '){
  int32_t r;
  String p = str_pull(s, pos, dlm);
  if (p==String("")) return false;
  if (!isUint(p)) return false;
  r = p.toInt();
  if (r<0) return false;
  if (r>65535) return false;
  *ret = r;
  return true;
}

// ******************************************************************************** / 
// get the part n in the form of uint32_t. return false if the part is not an uint32_t
// ******************************************************************************** / 
bool str_pull(String s, uint8_t pos, uint32_t* ret, char dlm = ' '){
  uint32_t r;
  String p = str_pull(s, pos, dlm);
  if (p==String("")) return false;
  if (!isUint(p)) return false;
  r = p.toInt();
  *ret = r;
  return true;
}

// ******************************************************************************** / 
// get the part n in the form of float. return false if the part is not a float
// ******************************************************************************** / 
bool str_pull(String s, uint8_t pos, float* ret, char dlm = ' '){
  float r;
  String p = str_pull(s, pos, dlm);
  if (p==String("")) return false;
  if (!isFloat(p)) return false;
  r = p.toFloat();
  *ret = r;
  return true;
}

// ******************************************************************************** / 
// frequency command
// ******************************************************************************** / 
bool cmd_FREQ(String args){
  float freq;
  if (!str_pull(args, 0, &freq)) {
    Serial.printf("FREQ error: not numeric value!\n");
    return false;
  } else {
    if ((freq>12500000) || (freq<=0)) {
      Serial.printf("FREQ error: frequency must be >0 and <12.500.000 Hz!\n");
      return false;
    } else {
      Serial.printf("FREQ: %.2f Hz\n", freq);     // WARN: this requires compilation with float printf support
    }
  }
  st.frequency = freq;
  dds.setFrequency(0, st.frequency);
  return true;
}

// ******************************************************************************** / 
// phase command
// ******************************************************************************** / 
bool cmd_PHASE(String args){
  uint16_t phase;
  if (!str_pull(args, 0, &phase)) {
    Serial.printf("PHASE error: not numeric value!\n");
    return false;
  } else {
    if ((phase>65535)) {
      Serial.printf("PHASE error: phase must be > 0!\n");
      return false;
    } else {
      Serial.printf("PHASE: %.1f deg\n", (float)phase/10.0);
    }
  }
  st.phase = phase;
  dds.setPhase(0, (float)st.phase/10.0);
  return true;
}

// ******************************************************************************** / 
// shape command
// ******************************************************************************** / 
bool cmd_SHAPE(String args){
  String param = str_pull(args, 0);
  
  if (param == String("OFF")) {
    st.shape = AD_OFF;
  } else if (param == String("SINE")) {
    st.shape = AD_SINE;
  } else if (param == String("SQUARE")) {
    st.shape = AD_SQUARE;  
  } else if (param == String("SQUARE2")) {
    st.shape = AD_SQUARE2;  
  } else if (param == String("TRIANGLE")) {
    st.shape = AD_TRIANGLE;  
  } else {
    Serial.printf("shape type unknown!\n");
    return false;
  }
  dds.setShape(st.shape);
  return true;
}

// ******************************************************************************** / 
// phase command
// ******************************************************************************** / 
bool cmd_MCLOCK(String args){
  uint32_t masterClock;
  if (!str_pull(args, 0, &masterClock)) {
    Serial.printf("MCLOCK error: not numeric value!\n");
    return false;
  } else {
    if ((masterClock>26000000) || (masterClock == 0)) {
      Serial.printf("MCLOCK error: must be between 1 and 26.000.000 Hz!\n");
      return false;
    } else {
      Serial.printf("MCLOCK: %lu HZ\n", masterClock);
    }
  }
  dds.masterClock(masterClock);         // adjust masterClock frequency
  dds.setFrequency(0, st.frequency);    // set frequency 
  return true;
}


// ******************************************************************************** / 
// help command
// ******************************************************************************** / 
bool cmd_HELP(String args){
  Serial.printf("HELP    this help\n");
  Serial.printf("FREQ    set frequency. Usage FREQ <Hz>\n");
  Serial.printf("PHASE   set phase. Usage PHASE <tenth of degs>\n");
  Serial.printf("SHAPE   set waveform. Usage SHAPE <off|sine|square|triangle|square2>\n");
  Serial.printf("MCLOCK  set master clock input frequency. Usage MCLOCK <Hz>. You have to specify the XTAL frequency here (i.e. 25000000)\n");
  return true;
}

// ******************************************************************************** / 
// command parser (BASE app)
// ******************************************************************************** / 
void parseCmd(){
  String cmd    = str_pull(cmdLine, 0);
  String args = cmdLine.substring(cmd.length()+1);
  bool ret = false;

  if      (cmd==String("HELP"))     ret = cmd_HELP(args);        
  else if (cmd==String("FREQ"))     ret = cmd_FREQ(args);         // COMMON commands
  else if (cmd==String("PHASE"))    ret = cmd_PHASE(args);
  else if (cmd==String("SHAPE"))    ret = cmd_SHAPE(args);
  else if (cmd==String("MCLOCK"))   ret = cmd_MCLOCK(args);
  else {
    Serial.printf("%s unknow command\n", cmd.c_str());
    Serial.printf("use HELP for a list of available commands\n");
    return;
  }
  if (ret) 
    Serial.printf("Command %s done\n", cmd.c_str());
  else
    Serial.printf("Command %s fail!\n", cmd.c_str());
}
