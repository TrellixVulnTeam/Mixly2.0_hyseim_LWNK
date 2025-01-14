'use strict';

goog.provide('Blockly.Python.actuator');
goog.require('Blockly.Python');

Blockly.Python.microbit_music_play_built_in = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    var dropdown_melody = block.getFieldValue('melody');
    var pin = Blockly.Python.valueToCode(block, 'PIN', Blockly.Python.ORDER_ATOMIC);
    var checkbox_wait = block.getFieldValue('wait') == 'TRUE' ? 'True' : 'False';
    var checkbox_loop = block.getFieldValue('loop') == 'TRUE' ? 'True' : 'False';
    var code = 'music.play(music.' + dropdown_melody + ', pin=pin' + pin +', wait=' + checkbox_wait + ', loop=' + checkbox_loop + ')\n';
    return code;
};

Blockly.Python.microbit_music_play_built_in_easy = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    var dropdown_melody = block.getFieldValue('melody');
    var pin = Blockly.Python.valueToCode(block, 'PIN', Blockly.Python.ORDER_ATOMIC);
    var code = 'music.play(music.' + dropdown_melody + ', pin=pin' + pin +')\n';
    return code;
};

Blockly.Python.microbit_music_pitch_delay = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    Blockly.Python.definitions_['import_math'] = 'import math';
    var number_pitch = Blockly.Python.valueToCode(block, 'pitch', Blockly.Python.ORDER_ATOMIC);
    var number_duration = Blockly.Python.valueToCode(block, 'duration', Blockly.Python.ORDER_ATOMIC);
    var pin = Blockly.Python.valueToCode(block, 'PIN', Blockly.Python.ORDER_ATOMIC);
    var checkbox_wait = block.getFieldValue('wait') == 'TRUE' ? 'True' : 'False';
    var code = 'music.pitch(round(' + number_pitch + '), round(' + number_duration + '), pin' + pin + ', wait = ' + checkbox_wait + ')\n';
    return code;
};

Blockly.Python.microbit_music_pitch = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    Blockly.Python.definitions_['import_math'] = 'import math';
    var number_pitch = Blockly.Python.valueToCode(block, 'pitch', Blockly.Python.ORDER_ATOMIC);
    // var number_duration = Blockly.Python.valueToCode(block, 'duration', Blockly.Python.ORDER_ATOMIC);
    var pin = Blockly.Python.valueToCode(block, 'PIN', Blockly.Python.ORDER_ATOMIC);
    var code = 'music.pitch(round(' + number_pitch + '), pin=pin' + pin + ')\n';
    return code;
};

Blockly.Python.microbit_music_play_list_of_notes = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    var pin = Blockly.Python.valueToCode(block, 'PIN', Blockly.Python.ORDER_ATOMIC);
    var value_notes = Blockly.Python.valueToCode(block, 'notes', Blockly.Python.ORDER_ATOMIC) ||'[]';
    var checkbox_wait = block.getFieldValue('wait') == 'TRUE' ? 'True' : 'False';
    var checkbox_loop = block.getFieldValue('loop') == 'TRUE' ? 'True' : 'False';
    var code = 'music.play(' + value_notes + ', pin=pin' + pin + ', wait=' + checkbox_wait + ', loop=' + checkbox_loop + ')\n';
    return code;
};



Blockly.Python.microbit_music_reset = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    var code = 'music.reset()\n';
    return code;
};

Blockly.Python.microbit_music_stop = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    var pin = Blockly.Python.valueToCode(block, 'PIN', Blockly.Python.ORDER_ATOMIC);
    var code = 'music.stop(pin' + pin + ')\n';
    return code;
};

Blockly.Python.microbit_music_get_tempo = function(block) {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_music'] = 'import music';
    var code = 'music.get_tempo()';
    return [code, Blockly.Python.ORDER_ATOMIC];
};

Blockly.Python.tone_set_tempo=function(){
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    var bpm = Blockly.Python.valueToCode(this, 'BPM', Blockly.Python.ORDER_ASSIGNMENT);
    var ticks = Blockly.Python.valueToCode(this, 'TICKS', Blockly.Python.ORDER_ASSIGNMENT);
    var code = "music.set_tempo(ticks="+ ticks +", bpm="+ bpm +")\n";
    return code;
};

Blockly.Python.speech_translate=function(){
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_speech'] = 'import speech';
    var text = Blockly.Python.valueToCode(this, 'VAR', Blockly.Python.ORDER_ATOMIC);
    var code = ["speech.translate("+ text +")", Blockly.Python.ORDER_ATOMIC];
    return code
};

Blockly.Python.speech_say=function(){
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_speech'] = 'import speech';
    var mode = this.getFieldValue("MODE");
    var text = Blockly.Python.valueToCode(this, 'VAR', Blockly.Python.ORDER_ATOMIC);
    var pitch = Blockly.Python.valueToCode(this, 'pitch', Blockly.Python.ORDER_ATOMIC);
    var speed = Blockly.Python.valueToCode(this, 'speed', Blockly.Python.ORDER_ATOMIC);
    var mouth = Blockly.Python.valueToCode(this, 'mouth', Blockly.Python.ORDER_ATOMIC);
    var throat = Blockly.Python.valueToCode(this, 'throat', Blockly.Python.ORDER_ATOMIC);
    var code = "speech."+mode+"("+ text +", pitch="+pitch+", speed="+speed+", mouth="+mouth+", throat="+throat+")\n";
    return code
};

// Blockly.Python.speech_sing=function(){
//   Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
//   Blockly.Python.definitions_['import_speech'] = 'import speech';
//   var text = Blockly.Python.valueToCode(this, 'VAR', Blockly.Python.ORDER_ATOMIC);
//   var pitch = Blockly.Python.valueToCode(this, 'pitch', Blockly.Python.ORDER_ATOMIC);
//   var speed = Blockly.Python.valueToCode(this, 'speed', Blockly.Python.ORDER_ATOMIC);
//   var mouth = Blockly.Python.valueToCode(this, 'mouth', Blockly.Python.ORDER_ATOMIC);
//   var throat = Blockly.Python.valueToCode(this, 'throat', Blockly.Python.ORDER_ATOMIC);
//   var code = "speech.sing("+ text +", pitch="+pitch+", speed="+speed+", mouth="+mouth+", throat="+throat+")\n";
//   return code
// };


// Blockly.Python.speech_prenounce=function(){
//   Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
//   Blockly.Python.definitions_['import_speech'] = 'import speech';
//   var text = Blockly.Python.valueToCode(this, 'VAR', Blockly.Python.ORDER_ATOMIC);
//   var pitch = Blockly.Python.valueToCode(this, 'pitch', Blockly.Python.ORDER_ATOMIC);
//   var speed = Blockly.Python.valueToCode(this, 'speed', Blockly.Python.ORDER_ATOMIC);
//   var mouth = Blockly.Python.valueToCode(this, 'mouth', Blockly.Python.ORDER_ATOMIC);
//   var throat = Blockly.Python.valueToCode(this, 'throat', Blockly.Python.ORDER_ATOMIC);
//   var code = "speech.pronounce("+ text +", pitch="+pitch+", speed="+speed+", mouth="+mouth+", throat="+throat+")\n";
//   return code
// };

Blockly.Python.speech_say_easy=function(){
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_speech'] = 'import speech';
    var text = Blockly.Python.valueToCode(this, 'VAR', Blockly.Python.ORDER_ATOMIC);
    var code = "speech.say("+ text +")\n";
    return code
};

Blockly.Python.speech_sing_easy=function(){
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_speech'] = 'import speech';
    var text = Blockly.Python.valueToCode(this, 'VAR', Blockly.Python.ORDER_ATOMIC);
    var code = "speech.sing("+ text +")\n";
    return code
};


Blockly.Python.speech_pronounce_easy=function(){
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_speech'] = 'import speech';
    var text = Blockly.Python.valueToCode(this, 'VAR', Blockly.Python.ORDER_ATOMIC);
    var code = "speech.pronounce("+ text +")\n";
    return code
};

Blockly.Python.servo_move = function() {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_Servolib'] = 'import Servo';
    var dropdown_pin = Blockly.Python.valueToCode(this, 'PIN',Blockly.Python.ORDER_ATOMIC);
    var value_degree = Blockly.Python.valueToCode(this, 'DEGREE', Blockly.Python.ORDER_ATOMIC);
    if (!isNaN(parseInt(dropdown_pin)))
        var code = 'Servo.angle(pin'+dropdown_pin+', '+value_degree+')\n';
    else
        var code = 'Servo.angle('+dropdown_pin+', '+value_degree+')\n';
    return code;
};

Blockly.Python.bit_motor_control = function() {
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_motor_control'] = 'import motor_control';

    var Motor= this.getFieldValue('Motor');
    var mode= this.getFieldValue('mode');
    var speed = Blockly.Python.valueToCode(this, 'speed', Blockly.Python.ORDER_ATOMIC);

    var code = 'motor_control.MotorRun('+Motor+', '+mode+''+speed+')\n';
    return code;
};

Blockly.Python.display_rgb_init=function(){
    var dropdown_rgbpin = Blockly.Python.valueToCode(this, 'PIN', Blockly.Python.ORDER_ATOMIC);
    var value_ledcount = Blockly.Python.valueToCode(this, 'LEDCOUNT', Blockly.Python.ORDER_ATOMIC);
    Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
    Blockly.Python.definitions_['import_neopixel'] = 'import neopixel';
    // Blockly.Python.definitions_['include_display'] = '#include "Mixly.h"';
    Blockly.Python.setups_['var_rgb_display' + dropdown_rgbpin] = 'np = neopixel.NeoPixel(pin' + dropdown_rgbpin +  ', ' + value_ledcount + ')\n';
    // Blockly.Python.setups_['setup_rgb_display_begin_' + dropdown_rgbpin] = 'rgb_display_' + dropdown_rgbpin + '.begin();';
    // Blockly.Python.setups_['setup_rgb_display_setpin' + dropdown_rgbpin] = 'rgb_display_' + dropdown_rgbpin + '.setPin(' + dropdown_rgbpin + ');';
    return '';
};
Blockly.Python.display_rgb=function(){

  var value_led = Blockly.Python.valueToCode(this, '_LED_', Blockly.Python.ORDER_ATOMIC);
  var value_rvalue = Blockly.Python.valueToCode(this, 'RVALUE', Blockly.Python.ORDER_ATOMIC);
  var value_gvalue = Blockly.Python.valueToCode(this, 'GVALUE', Blockly.Python.ORDER_ATOMIC);
  var value_bvalue = Blockly.Python.valueToCode(this, 'BVALUE', Blockly.Python.ORDER_ATOMIC);
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_neopixel'] = 'import neopixel';
  var code ='np[' + value_led + '] = (' + value_rvalue + ', ' + value_gvalue + ', ' + value_bvalue + ')\n';
  return code;
};

Blockly.Python.display_rgb2=function(){
  var value_led = Blockly.Python.valueToCode(this, '_LED_', Blockly.Python.ORDER_ATOMIC);
  var colour_rgb_led_color = this.getFieldValue('RGB_LED_COLOR');
  var color = goog.color.hexToRgb(colour_rgb_led_color);
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_neopixel'] = 'import neopixel';
  var code = 'np['+value_led+'] = ('+color+')\n';
  code+='np.show()\n';
  return code;
};

Blockly.Python.display_rgb_show = function(){
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_neopixel'] = 'import neopixel';
  var code= 'np.show()\n';   
  return code;
};

Blockly.Python.MP3_INIT = function () {
  var dropdown_pin1 = Blockly.Python.valueToCode(this, 'RX',Blockly.Python.ORDER_ATOMIC);
  var dropdown_pin2 = Blockly.Python.valueToCode(this, 'TX',Blockly.Python.ORDER_ATOMIC);
  if (!isNaN(parseInt(dropdown_pin1)))
    dropdown_pin1 = "pin" + dropdown_pin1;
  if (!isNaN(parseInt(dropdown_pin2)))
    dropdown_pin2 = "pin" + dropdown_pin2;
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_QJ00X_MP3'] = 'from MP3 import QJ00X_MP3';
  var code = 'mp3' + ' = ' + 'QJ00X_MP3(mp3_rx='+dropdown_pin1+', mp3_tx='+dropdown_pin2+')\n';
  return code;
};

//mp3 控制播放
Blockly.Python.MP3_CONTROL = function () {
  var CONTROL_TYPE = this.getFieldValue('CONTROL_TYPE');
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_QJ00X_MP3'] = 'from MP3 import QJ00X_MP3';
  var code = 'mp3' + '.' + CONTROL_TYPE + '()\n';
  return code;
};

//mp3 循环模式
Blockly.Python.MP3_LOOP_MODE = function () {
  var LOOP_MODE = this.getFieldValue('LOOP_MODE');
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_QJ00X_MP3'] = 'from MP3 import QJ00X_MP3';
  var code = 'mp3' + '.set_loop(' + LOOP_MODE + ')\n';
  return code;
};

//mp3 EQ模式
Blockly.Python.MP3_EQ_MODE = function () {
  var EQ_MODE = this.getFieldValue('EQ_MODE');
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_QJ00X_MP3'] = 'from MP3 import QJ00X_MP3';
  var code = 'mp3' + '.set_eq(' + EQ_MODE + ')\n';
  return code;
};

//mp3 设置音量
Blockly.Python.MP3_VOL = function () {
  var vol = Blockly.Python.valueToCode(this, 'vol', Blockly.Python.ORDER_ATOMIC);
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_QJ00X_MP3'] = 'from MP3 import QJ00X_MP3';
  var code = 'mp3' + '.set_vol(' + vol + ')\n';
  return code;
};

//mp3 播放第N首
Blockly.Python.MP3_PLAY_NUM = function () {
  var NUM = Blockly.Python.valueToCode(this, 'NUM', Blockly.Python.ORDER_ATOMIC);
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_QJ00X_MP3'] = 'from MP3 import QJ00X_MP3';
  var code = 'mp3' + '.playFileByIndexNumber(' + NUM + ')\n';
  return code;
};

Blockly.Python.MP3_PLAY_FOLDER = function () {
  var FOLDER = Blockly.Python.valueToCode(this, 'FOLDER', Blockly.Python.ORDER_ATOMIC);
  var NUM = Blockly.Python.valueToCode(this, 'NUM', Blockly.Python.ORDER_ATOMIC);
  Blockly.Python.definitions_['import_microbit_*'] = 'from microbit import *';
  Blockly.Python.definitions_['import_QJ00X_MP3'] = 'from MP3 import QJ00X_MP3';
  var code = 'mp3' + '.set_folder('+FOLDER+', ' + NUM + ')\n';
  return code;
};