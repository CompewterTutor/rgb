// Выполняется при нажатии на кнопку
void Tach_0() {
 static unsigned long millis_prev;
 // Устроняем дребезг контакта
 if (millis() - 100 > millis_prev) {
  chaing = 1; // Выстовляем признак нажатия кнопки
 }
 millis_prev = millis();
}

void Time01() {
  tickerSetLow.attach(TimeLed*60, setT1, 0);
  LedON(r, g, b);
    chaing = 1;
     digitalWrite(buzer_pin,1);

}

void setT1(int state) {
 tickerSetLow.detach();
  digitalWrite(buzer_pin,0);
 chaing = 1;

}

