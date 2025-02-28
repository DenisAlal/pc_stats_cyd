[![latest](https://img.shields.io/github/v/release/GyverLibs/GyverNTP.svg?color=brightgreen)](https://github.com/GyverLibs/GyverNTP/releases/latest/download/GyverNTP.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GyverNTP.svg)](https://registry.platformio.org/libraries/gyverlibs/GyverNTP)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GyverNTP?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GyverNTP
Библиотека для получения точного времени с NTP сервера для esp8266/esp32
- Работает на стандартном интерфейсе Udp.h
- Учёт времени ответа сервера и задержки соединения
- Получение времени с точностью до миллисекунд
- Интеграция с библиотекой [Stamp](https://github.com/GyverLibs/Stamp) для распаковки unix в часы, минуты итд
- Автоматическая синхронизация
- Поддержание хода времени на базе millis() между синхронизациями
- Секундный таймер для удобства автоматизации
- Обработка ошибок
- Асинхронный режим
- Поддержка внешнего RTC
- Кеширование DNS, стабильная работа и возможность проверки наличия Интернет

### Совместимость
Все платформы

### Зависимости
- [Stamp](https://github.com/GyverLibs/Stamp) v1.4.0+

## Содержание
- [Инициализация](#init)
- [Использование](#usage)
- [Пример](#example)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="init"></a>

## Инициализация
```cpp
GyverNTP;                 // параметры по умолчанию (gmt 0, период 3600 секунд (1 час))
GyverNTP(gmt);            // часовой пояс в часах (например Москва 3)
GyverNTP(gmt, period);    // часовой пояс в часах и период обновления в секундах
```

> Начиная с версии 2.1.0 доступен глобальный объект `NTP`, свой создавать не нужно

<a id="usage"></a>

## Использование
```cpp
// установить часовой пояс в часах или минутах (глобально для Stamp)
void setGMT(int16_t gmt);

// установить период обновления в секундах
void setPeriod(uint16_t prd);

// включить асинхронный режим (по умолч. true)
void asyncMode(bool async);

// установить хост (умолч. "pool.ntp.org")
void setHost(const String& host);

// установить хост IP
void setHost(const IPAddress& host);

// установить порт (умолч. 123)
void setPort(uint16_t port);

// получить пинг NTP сервера, мс
int16_t ping();

// вернёт true при изменении статуса online
bool statusChanged();

// не учитывать пинг соединения (умолч. false)
void ignorePing(bool ignore) {
    _usePing = ignore;
}

// подключить RTC
void attachRTC(VirtualRTC& rtc);

// отключить RTC
void detachRTC();

// подключить обработчик ошибки
void onError(ErrorCallback cb);

// получить последнюю ошибку
Error getError();

// получить последнюю ошибку
const __FlashStringHelper* readError();

// есть ошибка
bool hasError();

// вернёт true, если tick ожидает ответа сервера в асинхронном режиме
bool busy();

// true - есть соединение с Интернет
bool online();

// запустить
bool begin();

// запустить с указанием часового пояса в часах или минутах (глобально для Stamp)
bool begin(int16_t gmt);

// выключить NTP
void end();

// синхронно обновить время с сервера. true при успехе
bool updateNow();

// тикер, вызывать в loop. Вернёт true каждую секунду, если синхронизирован. Синхронизируется по таймеру
bool tick();
```

## Особенности
- GyverNTP работает с WiFi UDP для esp8266/esp32, но может использоваться любой другой UDP клиент (см. ниже)
- Существует глобальный объект `NTP` (как `Serial`, `Wire` И проч.)
- Нужно вызывать `tick()` в главном цикле программы `loop()`, он синхронизирует время с сервера по своему таймеру и обеспечивает работу секундного таймера
- Если основной цикл программы сильно загружен, а время нужно получать с максимальной точностью (несколько мс), то можно выключить асинхронный режим `asyncMode(false)`
- Библиотека продолжает считать время после пропадания синхронизации. По моим тестам esp "уходит" на ~1.7 секунды за сутки, поэтому стандартный период синхронизации выбран 1 час
- Наследуется класс `StampKeeper`, который обеспечивает счёт времени, работу секундного таймера и удобную конвертацию времени

### Часовой пояс
Часовой пояс задаётся для всех операций со Stamp/Datime в программе! Установка часового пояса в объекте NTP равносильна вызову `setStampZone()` - установка **глобального** часового пояса для библиотеки Stamp:

```cpp
void setup() {
    // подключить к WiFi
    NTP.begin(3); // запустить и указать часовой пояс
}
```

### Минимальный пример
```cpp
#include <GyverNTP.h>

void setup() {
    // подключить к WiFi
    NTP.begin(3); // запустить и указать часовой пояс
}

void loop() {
    NTP.tick();   // вызывать тикер в loop
}
```

### Секундный таймер
Для удобства автоматизации событий по таймеру в библиотеку встроен секундный таймер, он срабатывает в 0 миллисекунд каждой секунды. По условию таймера NTP гарантированно синхронизирован и выдаёт корректное время:

```cpp
void loop() {
  if (NTP.tick()) {
    // новая секунда!
    Serial.println(NTP.toString());
  }

  // или так
  // NTP.tick();
  // if (NTP.newSecond()) { }
}
```

Также можно подключить обработчик на секунду:
```cpp
void newSecond() {
  // ваш код
}

void setup() {
  // подключить к WiFi
  NTP.begin(3); // запустить и указать часовой пояс
  NTP.onSecond(newSecond);

  // или так
  NTP.onSecond([](){
    // ваш код
  });
}

void loop() {
  NTP.tick();
}
```

### Получение времени
GyverNTP наследует [StampConvert](https://github.com/GyverLibs/Stamp), то есть получать время можно множеством способов:

```cpp
// каждую секунду
if (NTP.tick()) {
  // вывод даты и времени строкой
  Serial.print(NTP.toString());  // NTP.timeToString(), NTP.dateToString()
  Serial.print(':');
  Serial.println(NTP.ms());  // + миллисекунды текущей секунды. Внутри tick всегда равно 0

  // вывод в Datime
  Datime dt = NTP;  // или Datime dt(NTP)
  dt.year;
  dt.second;
  dt.hour;
  dt.weekDay;
  dt.yearDay;
  // ... и прочие методы и переменные Datime

  // чтение напрямую, медленнее чем вывод в Datime
  NTP.second();
  NTP.minute();
  NTP.year();
  // ... и прочие методы StampConvert

  // сравнение
  NTP == DaySeconds(12, 35, 0);            // сравнение с DaySeconds (время равно 12:35:00)
  NTP == 1738237474;                       // сравнение с unix
  NTP == Datime(2025, 1, 30, 14, 14, 30);  // сравнение с Datime
}
```

### Режим синхронизации
GyverNTP может использоваться и **без тикера** - нужно вручную вызвать `updateNow` для синхронизации времени. В этом случае время будет считаться просто с момента последней синхронизации, обработчик секунд не будет работать:

```cpp
void setup() {
  // подключить к WiFi
  NTP.begin(3);     // запустить и указать часовой пояс
  NTP.updateNow();  // синхронизировать
}

void loop() {
  Serial.println(NTP.toString());
  delay(1000);
}
```

### Рассинхронизация
Если период синхронизации очень большой или в системе надолго пропадает связь, часы рассинхронизируются и будут синхронизированы при следующем обращении к серверу. Если время ушло больше, чем на 1 секунду, то поведение будет следующим:
- Если внутренние часы "спешат" - секундный таймер перестанет срабатывать, пока реальное время не догонит внутреннее
- Если внутренние часы "отстают" - таймер будет вызываться каждую итерацию loop с прибавлением времени, пока внутреннее время не догонит реальное

Это сделано для того, чтобы при синхронизации не потерялись секунды - библиотека обработает каждую секунду и не будет повторяться, что очень важно для алгоритмов автоматизации.

### Проверка онлайна
NTP работает по UDP - очень легковесному и "дешёвому" протоколу связи, обращение к серверу практически не занимает времени. Благодаря этому NTP можно использовать для проверки связи с Интернет - там, где стандартный TCP клиент зависнет на несколько секунд, NTP асинхронно сообщит о потере связи. В рамках GyverNTP это можно использовать так:
```cpp
void setup() {
  // подключить к WiFi
  NTP.begin(3);
  NTP.setPeriod(5); // синхронизация каждые 5 секунд
}

void loop() {
  NTP.tick();

  // вернёт true при смене статуса
  if (NTP.statusChanged()) {
    Serial.println(NTP.online());
    // здесь флаг online можно использовать для передачи в другие библиотеки
    // например FastBot2
    // bot.setOnline(NTP.online());
  }
}
```

### Подключение RTC
Библиотека поддерживает подключение внешнего RTC для синхронизации времени:
- Если пришло время синхронизироваться с NTP, но возникла ошибка - будет синхронизировано время с RTC
- Если успешно синхронизирован с NTP, то в RTC также будет записано актуальное время (не чаще чем `GNTP_RTC_WRITE_PERIOD`, по умолч. 1 час)

Объект RTC должен являться экземпляром класса `VirtualRTC`, из готовых например `GyverDS3231Min`. Можно написать и свой класс для подключения любого источника реального времени.

<a id="example"></a>

## Пример
### Полное демо
```cpp
#include <Arduino.h>
#include <GyverNTP.h>

void setup() {
    Serial.begin(115200);
    WiFi.begin("WIFI_SSID", "WIFI_PASS");
    while (WiFi.status() != WL_CONNECTED) delay(100);
    Serial.println("Connected");

    // обработчик ошибок
    NTP.onError([]() {
        Serial.println(NTP.readError());
        Serial.print("online: ");
        Serial.println(NTP.online());
    });

    // обработчик секунды (вызывается из тикера)
    NTP.onSecond([]() {
        Serial.println("new second!");
    });

    // обработчик синхронизации (вызывается из sync)
    // NTP.onSync([](uint32_t unix) {
    //     Serial.println("sync: ");
    //     Serial.print(unix);
    // });

    NTP.begin(3);                           // запустить и указать часовой пояс
    // NTP.setPeriod(30);                   // период синхронизации в секундах
    // NTP.setHost("ntp1.stratum2.ru");     // установить другой хост
    // NTP.setHost(IPAddress(1, 2, 3, 4));  // установить другой хост
    // NTP.asyncMode(false);                // выключить асинхронный режим
    // NTP.ignorePing(true);                // не учитывать пинг до сервера
    // NTP.updateNow();                     // обновить прямо сейчас
}

void loop() {
    // тикер вернёт true каждую секунду в 0 мс секунды, если время синхронизировано
    if (NTP.tick()) {
        // вывод даты и времени строкой
        Serial.print(NTP.toString());  // NTP.timeToString(), NTP.dateToString()
        Serial.print(':');
        Serial.println(NTP.ms());  // + миллисекунды текущей секунды. Внутри tick всегда равно 0

        // вывод в Datime
        Datime dt = NTP;  // или Datime dt(NTP)
        dt.year;
        dt.second;
        dt.hour;
        dt.weekDay;
        dt.yearDay;
        // ... и прочие методы и переменные Datime

        // чтение напрямую, медленнее чем вывод в Datime
        NTP.second();
        NTP.minute();
        NTP.year();
        // ... и прочие методы StampConvert

        // сравнение
        NTP == DaySeconds(12, 35, 0);            // сравнение с DaySeconds (время равно 12:35:00)
        NTP == 1738237474;                       // сравнение с unix
        NTP == Datime(2025, 1, 30, 14, 14, 30);  // сравнение с Datime
    }

    if (NTP.newSecond()) {
        // новую секунду можно поймать и здесь
    }

    // изменился онлайн-статус
    if (NTP.statusChanged()) {
        Serial.print("STATUS: ");
        Serial.println(NTP.online());
    }
}
```

### RTC

```cpp
#include <Arduino.h>
#include <GyverNTP.h>

// GyverDS3231 поддерживает работу с GyverNTP
#include <GyverDS3231Min.h>
GyverDS3231Min rtc;

// можно написать свой класс и использовать любой другой RTC
class RTC : public VirtualRTC {
   public:
    void setUnix(uint32_t unix) {
        Serial.print("SET RTC: ");
        Serial.println(unix);
    }
    uint32_t getUnix() {
        return 1738015299ul;
    }
};
RTC vrtc;

void setup() {
    Serial.begin(115200);
    WiFi.begin("WIFI_SSID", "WIFI_PASS");
    // while (WiFi.status() != WL_CONNECTED) delay(100);
    Serial.println("Connected");

    // GyverDS3231
    Wire.begin();
    rtc.begin();

    NTP.begin(3);  // запустить и указать часовой пояс

    // подключить RTC
    // NTP.attachRTC(vrtc);
    NTP.attachRTC(rtc);
}

void loop() {
    if (NTP.tick()) {
        Serial.println(NTP.toString());
    }
}
```

<a id="versions"></a>

## Версии
- v1.0
- v1.1 - мелкие улучшения и gmt в минутах
- v1.2 - оптимизация, улучшена стабильность, добавлен асинхронный режим
- v1.2.1 - изменён стандартный период обновления
- v1.3 - ускорена синхронизация при запуске в асинхронном режиме
- v1.3.1 - заинклудил WiFi библиотеку в файл
- v2.0 - добавлена зависимость от Stamp, больше возможностей, проверка онлайна для других библиотек
- v2.1 - добавлен глобальный объект NTP
- v2.2.0 - более стабильная работа, новые возможности

<a id="install"></a>

## Установка
- Библиотеку можно найти по названию **GyverNTP** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/GyverNTP/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>

## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!


При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код
