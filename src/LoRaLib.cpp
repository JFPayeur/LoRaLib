#include "LoRaLib.h"

LoRa::LoRa(Chip ch, int nss, float freq, Bandwidth bw, SpreadingFactor sf, CodingRate cr, int dio0, int dio1) {
  dataRate = 0;
  lastPacketRSSI = 0;
  
  switch(ch) {
    case CH_SX1272:
      _mod = new SX1272(nss, freq, bw, sf, cr, dio0, dio1);
      break;
    case CH_SX1273:
      _mod = new SX1273(nss, freq, bw, sf, cr, dio0, dio1);
      break;
    case CH_SX1276:
      _mod = new SX1276(nss, freq, bw, sf, cr, dio0, dio1);
      break;
    case CH_SX1277:
      _mod = new SX1277(nss, freq, bw, sf, cr, dio0, dio1);
      break;
    case CH_SX1278:
      _mod = new SX1278(nss, freq, bw, sf, cr, dio0, dio1);
      break;
    case CH_SX1279:
      _mod = new SX1279(nss, freq, bw, sf, cr, dio0, dio1);
      break;
  }
}

uint8_t LoRa::begin(uint16_t addrEeprom) {
  #ifdef DEBUG
    Serial.begin(9600);
    Serial.println();
  #endif
  
  #ifdef ESP32
    if(!EEPROM.begin(9)) {
      #ifdef DEBUG
        Serial.println("Unable to initialize EEPROM");
      #endif
      return(ERR_EEPROM_NOT_INITIALIZED);
    }
  #endif
  
  _addrEeprom = addrEeprom;
  
  bool hasAddress = false;
  for(uint16_t i = 0; i < 8; i++) {
    if(EEPROM.read(_addrEeprom + i) != 255) {
      hasAddress = true;
      break;
    }
  }
  
  if(!hasAddress) {
    randomSeed(analogRead(5));
    generateLoRaAdress();
  }
  
  #ifdef DEBUG
    Serial.print("LoRa node address string: ");
  #endif
  for(uint8_t i = 0; i < 8; i++) {
    _address[i] = EEPROM.read(i);
    #ifdef DEBUG
      Serial.print(_address[i], HEX);
      if(i < 7) {
        Serial.print(":");
      } else {
        Serial.println();
      }
    #endif
  }
  
  return(_mod->begin());
}

uint8_t LoRa::transmit(Packet& pack) {
  char buffer[256];
  
  for(uint8_t i = 0; i < 8; i++) {
    buffer[i] = pack.source[i];
    buffer[i+8] = pack.destination[i];
  }
  
  for(uint8_t i = 0; i < pack.length; i++) {
    buffer[i+16] = pack.data[i];
  }
  
  return(_mod->tx(buffer, pack.length));
}

uint8_t LoRa::receive(Packet& pack) {
  char buffer[256];
  uint32_t startTime = millis();
  
  uint8_t status = _mod->rxSingle(buffer, &pack.length);
  
  for(uint8_t i = 0; i < 8; i++) {
    pack.source[i] = buffer[i];
    pack.destination[i] = buffer[i+8];
  }
  
  for(uint8_t i = 16; i < pack.length; i++) {
    pack.data[i-16] = buffer[i];
  }
  pack.data[pack.length-16] = 0;
  
  uint32_t elapsedTime = millis() - startTime;
  dataRate = (pack.length*8.0)/((float)elapsedTime/1000.0);
  lastPacketRSSI = _mod->getLastPacketRSSI();
  
  return(status);
}

uint8_t LoRa::sleep(void) {
  return(_mod->setMode(0b00000000));
}

uint8_t LoRa::standby(void) {
  return(_mod->setMode(0b00000001));
}

uint8_t LoRa::setBandwidth(Bandwidth bw) {
  return(_mod->setBandwidth(bw));
}

uint8_t LoRa::setSpreadingFactor(SpreadingFactor sf) {
  return(_mod->setSpreadingFactor(sf));
}

uint8_t LoRa::setCodingRate(CodingRate cr) {
  return(_mod->setCodingRate(cr));
}

uint8_t LoRa::setFrequency(float freq) {
  return(_mod->setFrequency(freq));
}

void LoRa::generateLoRaAdress(void) {
  for(uint8_t i = _addrEeprom; i < (_addrEeprom + 8); i++) {
    EEPROM.write(i, (uint8_t)random(0, 256));
  }
}
