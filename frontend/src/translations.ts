export type Lang = 'uk' | 'en'

export interface Translations {
  appTitle: string
  appSubtitle: string
  tabHome: string
  tabHomeSubtitle: string
  tabInfo: string
  tabSettings: string
  connecting: string

  deepSleepBanner: string
  batteryStatus: string
  untilNextFeed: string
  isCharging: string
  lowBattery: string
  voltageLabel: string

  manualFeed: string
  manualFeedSub: string
  repeatCount: string
  feedNow: string
  feedNowCooldown: (s: number) => string
  feedCooldownLabel: string
  feeding: string

  foodShowSection: string
  foodSettingsSub: string

  lightSensor: string
  lightSensorSub: string
  lightLuxLabel: string
  lightOn: string
  lightOff: string
  lightSettingsSub: string
  lightShowGauge: string
  lightThresholdLabel: string
  lightThresholdHint: string
  feedJournal: string
  feedJournalSub: string
  feedJournalEmpty: string
  feedJournalSource: (s: string) => string

  schedule: string
  scheduleSub: string
  nextFeedIn: (h: number, m: number) => string
  nextFeedLabel: (h: number, m: number) => string
  repeats: string
  day: string
  addFeeding: string
  saveAllTimes: string
  days: string[]

  manualControl: string
  manualControlSub: string
  servoAngle: (v: number) => string
  servoSpeed: (v: number) => string
  saveSpeed: string

  wifiInfo: string
  wifiInfoSub: string
  ssid: string
  ipAddr: string
  modeLabel: string
  mdns: string
  notConfigured: string
  notConnected: string
  apMode: string
  staMode: string

  batterySection: string
  batterySectionSub: string
  voltageValue: string
  percentValue: string

  settingsInfo: string
  settingsInfoSub: string
  servoSpeedLabel: string
  feedRepeatsLabel: string
  powerSaveLabel: string
  oledLabel: string
  scheduleCountLabel: string
  on: string
  off: string

  systemInfo: string
  systemInfoSub: string
  model: string
  firmwareVersion: string
  uptime: string
  uptimeFmt: (h: number, m: number) => string
  cpuFreq: string
  deviceTime: string

  memoryInfo: string
  memoryInfoSub: string
  freeMemory: string
  usedMemory: string
  totalMemory: string
  maxBlock: string
  minFree: string
  cacheSizeLabel: string
  cacheAgeLabel: string
  cacheStatusLabel: string
  cacheActive: string
  cacheInactive: string

  langTitle: string
  langSub: string

  wifiSettings: string
  wifiSettingsSub: string
  forgetNetwork: string
  forgetHint: string

  powerSettings: string
  powerSettingsSub: string
  powerSaveToggle: string
  powerSaveHint: string
  deepSleepLabel: string
  oledToggle: string
  oledHint: string
  displayOffLabel: (n: number) => string
  savePowerSettings: string

  feedInterval: string
  feedIntervalSub: string
  minIntervalLabel: string
  minIntervalHint: (n: number) => string
  saveInterval: string

  calibration: string
  calibrationSub: string
  calibrationCurrent: (v: string) => string
  calibrationLabel: string
  calibrationHint: string
  applyCalibration: string
  calibrationInvalid: string

  timezone: string
  timezoneSub: string
  timezoneLabel: string
  timezoneHint: string
  saveTimezone: string

  account: string
  accountSub: string
  signOut: string

  foodTitle: string
  foodRemaining: (g: number) => string
  foodDuration: (months: number, days: number) => string
  foodRefill: string
  foodGramsTotal: string
  foodGramsPerFeed: string
  foodSave: string
  foodCancel: string
  foodNotSet: string
  foodNoSchedule: string
  foodFeedingsPerDay: (n: number) => string

  tabStats: string
  statsFeedings: string
  statsFeedingsToday: string
  statsAvgPerDay: string
  statsLight: string
  statsLightToday: string
  statsLightHours: (h: number, m: number) => string
  statsRecommendations: string
  recNoFeedingsToday: string
  recFeedMore: string
  recFeedTooMuch: string
  recNoLight: string
  recLightTooShort: string
  recLightGood: string
  recAllGood: string
  dailyTips: string[]

  loginSignIn: string
  loginRegister: string
  loginEmail: string
  loginPassword: string
  loginSubtitle: string
  loginNetworkError: string
  loginLoading: string

  toastFeeding: string
  toastFed: string
  toastFeedError: string
  toastScheduleSaved: string
  toastSaveError: string
  toastSaved: string
  toastFeedAdded: string
  toastFeedRemoved: string
  toastPowerSave: (on: boolean) => string
  toastError: string
  toastIntervalSaved: string
  toastCalibDone: string
  toastCalibError: string
  toastTimezoneSaved: string
  toastCommandSent: string
}

const uk: Translations = {
  appTitle: 'AquaFeed Control',
  appSubtitle: 'Розумна годівниця',
  tabHome: 'Головна',
  tabHomeSubtitle: 'Керування годівницею',
  tabInfo: 'Інформація',
  tabSettings: 'Налаштування',
  connecting: 'Підключення...',

  deepSleepBanner: 'Якщо сторінка не відкривається — пристрій може бути в глибокому сні. Натисніть фізичну кнопку на корпусі, щоб прокинути та знову відкрити веб.',
  batteryStatus: 'Стан батареї',
  untilNextFeed: 'До наступного годування',
  isCharging: 'Заряджається',
  lowBattery: 'Низький заряд батареї',
  voltageLabel: 'Напруга:',

  manualFeed: 'Ручне годування',
  manualFeedSub: 'Швидкий запуск циклу годування',
  repeatCount: 'Кількість повторів',
  feedNow: 'Годувати зараз',
  feedNowCooldown: s => `Годувати зараз (${s} с)`,
  feedCooldownLabel: 'Наступне годування',
  feeding: 'Годую...',

  foodShowSection: 'Показувати запас корму',
  foodSettingsSub: 'Відображення секції запасу корму',

  lightSensor: 'Сенсор світла',
  lightSensorSub: 'Поточна освітленість',
  lightLuxLabel: 'Освітленість:',
  lightOn: 'Ввімкнено',
  lightOff: 'Вимкнено',
  lightSettingsSub: 'Показ та чутливість сенсора',
  lightShowGauge: 'Показувати gauge сенсора',
  lightThresholdLabel: 'Поріг «Ввімкнено» (лк):',
  lightThresholdHint: 'Якщо значення сенсора вище порогу — вважається що світло ввімкнено.',
  feedJournal: 'Журнал годувань',
  feedJournalSub: 'Останні 20 годувань',
  feedJournalEmpty: 'Немає записів',
  feedJournalSource: s => s === 'manual' ? 'Вручну' : s === 'scheduled' ? 'Розклад' : s,

  schedule: 'Автоматичне годування',
  scheduleSub: 'Налаштуйте розклад годувань',
  nextFeedIn: (h, m) => `${h} год ${m} хв`,
  nextFeedLabel: (h, m) => `До наступного годування: ${h} год ${m} хв`,
  repeats: 'Повторів:',
  day: 'День:',
  addFeeding: '+ Додати годування',
  saveAllTimes: 'Зберегти всі часи',
  days: ['Щодня', 'Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб', 'Нд'],

  manualControl: 'Ручне керування',
  manualControlSub: 'Ручне керування сервоприводом',
  servoAngle: v => `Кут серво: ${v}°`,
  servoSpeed: v => `Швидкість серво: ${v}`,
  saveSpeed: 'Зберегти швидкість',

  wifiInfo: 'WiFi інформація',
  wifiInfoSub: 'Поточні мережеві параметри',
  ssid: 'SSID:',
  ipAddr: 'IP адреса:',
  modeLabel: 'Режим:',
  mdns: 'mDNS:',
  notConfigured: 'не налаштовано',
  notConnected: 'не підключено',
  apMode: 'Точка доступу (AP)',
  staMode: 'Станція (STA)',

  batterySection: 'Батарея',
  batterySectionSub: 'Стан акумулятора пристрою',
  voltageValue: 'Напруга:',
  percentValue: 'Відсоток:',

  settingsInfo: 'Налаштування',
  settingsInfoSub: 'Поточні параметри роботи',
  servoSpeedLabel: 'Швидкість серво:',
  feedRepeatsLabel: 'Повторів годування:',
  powerSaveLabel: 'Режим економії:',
  oledLabel: 'OLED дисплей:',
  scheduleCountLabel: 'Кількість розкладів:',
  on: 'Увімкнено',
  off: 'Вимкнено',

  systemInfo: 'Система',
  systemInfoSub: 'Інформація про пристрій',
  model: 'Модель:',
  firmwareVersion: 'Версія прошивки:',
  uptime: 'Час роботи:',
  uptimeFmt: (h, m) => `${h} год ${m} хв`,
  cpuFreq: 'CPU частота:',
  deviceTime: 'Час пристрою:',

  memoryInfo: "Пам'ять та кеш",
  memoryInfoSub: 'Використання ресурсів системи',
  freeMemory: "Вільна пам'ять:",
  usedMemory: "Використано пам'яті:",
  totalMemory: "Загальна пам'ять:",
  maxBlock: 'Макс. блок:',
  minFree: 'Мін. вільна (завжди):',
  cacheSizeLabel: 'Розмір кешу:',
  cacheAgeLabel: 'Вік кешу:',
  cacheStatusLabel: 'Статус кешу:',
  cacheActive: '✅ Активний',
  cacheInactive: '❌ Неактивний',

  langTitle: 'Мова інтерфейсу',
  langSub: 'Оберіть мову веб-інтерфейсу',

  wifiSettings: 'Підключення до WiFi',
  wifiSettingsSub: 'Управління збереженою мережею',
  forgetNetwork: 'Забути мережу',
  forgetHint: 'Кнопка «Забути» видаляє збережені дані мережі та повертає пристрій у режим точки доступу. Підключіться до мережі FishFeeder-XXXX і відкрийте 192.168.4.1 для налаштування.',

  powerSettings: 'Налаштування енергії',
  powerSettingsSub: 'Оптимізуйте споживання живлення',
  powerSaveToggle: 'Режим збереження енергії',
  powerSaveHint: 'Режим економії увімкнено: після бездіяльності — глибокий сон. Пробудження перед розкладом (таймер) або кнопкою. Веб не дає заснути.',
  deepSleepLabel: 'Секунд бездіяльності до глибокого сну (10–3600):',
  oledToggle: 'Увімкнути OLED дисплей',
  oledHint: 'Вимкніть дисплей для економії енергії. Всі функції працюють через веб-інтерфейс.',
  displayOffLabel: n => `Секунд до вимкнення OLED (режим економії, 5–600): За замовчуванням ${n} с.`,
  savePowerSettings: 'Зберегти налаштування економії енергії',

  feedInterval: 'Інтервал годування',
  feedIntervalSub: 'Окреме обмеження частоти запуску годувань',
  minIntervalLabel: 'Мінімальний інтервал між годуваннями (хв, 1–1440):',
  minIntervalHint: n => `Це обмежує, як швидко після попереднього можна запустити нове годування. За замовчуванням ${n} хв.`,
  saveInterval: 'Зберегти інтервал годування',

  calibration: 'Калібрування батареї',
  calibrationSub: 'Ручне підлаштування напруги за мультиметром',
  calibrationCurrent: v => `Поточна напруга з сенсора: ${v} В`,
  calibrationLabel: 'Фактична напруга з мультиметра (В):',
  calibrationHint: "Введіть значення з мультиметра і застосуйте. Для прев'ю калібрування зберігається локально.",
  applyCalibration: 'Застосувати калібрування',
  calibrationInvalid: 'Введіть коректну напругу (2.5–4.5 В)',

  timezone: 'Часовий пояс',
  timezoneSub: 'Окремо керуйте локальним часом пристрою',
  timezoneLabel: 'Часовий пояс (зсув UTC):',
  timezoneHint: 'Оберіть локальний зсув UTC для годинника, розкладу та відліку до наступного годування. Якщо у вашому регіоні змінюється літній/зимовий час, оновлюйте це вручну.',
  saveTimezone: 'Зберегти часовий пояс',

  account: 'Акаунт',
  accountSub: 'Вийти з AquaFeed',
  signOut: 'Вийти',

  foodTitle: 'Запас корму',
  foodRemaining: g => `${g} г залишилось`,
  foodDuration: (m, d) => m > 0 ? `~${m} міс ${d} дн` : `~${d} дн`,
  foodRefill: 'Поповнити',
  foodGramsTotal: 'Завантажено (г)',
  foodGramsPerFeed: 'Грамів за 1 годівлю',
  foodSave: 'Зберегти',
  foodCancel: 'Скасувати',
  foodNotSet: 'Вкажіть кількість корму',
  foodNoSchedule: 'Немає активного розкладу',
  foodFeedingsPerDay: n => `${n} годівель/добу`,

  tabStats: 'Статистика',
  statsFeedings: 'Годування (останні 7 днів)',
  statsFeedingsToday: 'Сьогодні годувань',
  statsAvgPerDay: 'Середнє / день',
  statsLight: 'Тривалість світла (хв)',
  statsLightToday: 'Світло сьогодні',
  statsLightHours: (h, m) => h > 0 ? `${h} год ${m} хв` : `${m} хв`,
  statsRecommendations: 'Рекомендації',
  recNoFeedingsToday: 'Сьогодні годувань ще немає — час погодувати рибок!',
  recFeedMore: 'Годування відбуваються рідко. Рекомендується 2–4 рази на день.',
  recFeedTooMuch: 'Забагато годувань — надлишок корму погіршує якість води.',
  recNoLight: 'Сьогодні світло не вмикалось — рибкам потрібен денний цикл.',
  recLightTooShort: 'Світло горить менше 8 годин. Рекомендується 8–12 годин на день.',
  recLightGood: 'Тривалість світла оптимальна. Так тримати!',
  recAllGood: 'Все в нормі! Рибки щасливі.',
  dailyTips: [
    'Замінюйте 20–30% води щотижня — це підтримує баланс і чистоту акваріума.',
    'Перевіряйте фільтр раз на місяць: забитий фільтр погіршує якість води.',
    'Не перегодовуйте — залишки корму розкладаються і підвищують рівень аміаку.',
    'Температура води повинна бути стабільною: різкі перепади стресують рибок.',
    'Рослини в акваріумі поглинають нітрати і насичують воду киснем.',
    'Чистіть стінки акваріума від водоростей раз на тиждень.',
    'Після тривалого відключення світла перевірте поведінку рибок — вони мають бути активними.',
    'Нові рибки потребують 2 тижні карантину перед переміщенням в основний акваріум.',
    'Використовуйте кондиціонер для нейтралізації хлору при кожній заміні води.',
    'Регулярно перевіряйте рівень pH — для більшості риб оптимальний діапазон 6.5–7.5.',
    'Правильне освітлення 8–12 годин на день підтримує природний цикл для риб і рослин.',
    'Уникайте розміщення акваріума під прямим сонячним світлом — це спричиняє цвітіння водоростей.',
    'Годуйте рибок дрібними порціями 2–3 рази на день замість однієї великої.',
    'Рівень нітратів понад 40 мг/л шкідливий — частіша заміна води вирішує проблему.',
    'Перевіряйте аератор щомісяця: хороша аерація критична для здоров\'я риб.',
  ],

  loginSignIn: 'Увійти',
  loginRegister: 'Реєстрація',
  loginEmail: 'Email',
  loginPassword: 'Пароль',
  loginSubtitle: 'Автоматична годівниця',
  loginNetworkError: 'Помилка мережі. Спробуйте ще раз.',
  loginLoading: 'Зачекайте...',

  toastFeeding: 'Годую...',
  toastFed: 'Годування розпочато!',
  toastFeedError: 'Помилка годування',
  toastScheduleSaved: 'Розклад збережено',
  toastSaveError: 'Помилка збереження',
  toastSaved: 'Налаштування збережено',
  toastFeedAdded: 'Годування додано',
  toastFeedRemoved: 'Годування видалено',
  toastPowerSave: on => `Режим збереження ${on ? 'увімкнено' : 'вимкнено'}`,
  toastError: 'Помилка',
  toastIntervalSaved: 'Інтервал збережено',
  toastCalibDone: 'Калібрування виконано',
  toastCalibError: 'Помилка калібрування',
  toastTimezoneSaved: 'Часовий пояс збережено',
  toastCommandSent: 'Команду відправлено',
}

const en: Translations = {
  appTitle: 'AquaFeed Control',
  appSubtitle: 'Smart Feeder',
  tabHome: 'Home',
  tabHomeSubtitle: 'Feeder control',
  tabInfo: 'Info',
  tabSettings: 'Settings',
  connecting: 'Connecting...',

  deepSleepBanner: 'If the page does not open — the device may be in deep sleep. Press the physical button on the case to wake it and reopen the web interface.',
  batteryStatus: 'Battery Status',
  untilNextFeed: 'Until Next Feed',
  isCharging: 'Charging',
  lowBattery: 'Low Battery',
  voltageLabel: 'Voltage:',

  manualFeed: 'Manual Feed',
  manualFeedSub: 'Quick feeding cycle start',
  repeatCount: 'Number of repeats',
  feedNow: 'Feed Now',
  feedNowCooldown: s => `Feed Now (${s} s)`,
  feedCooldownLabel: 'Next feeding',
  feeding: 'Feeding...',

  foodShowSection: 'Show food supply section',
  foodSettingsSub: 'Show or hide the food supply widget',

  lightSensor: 'Light Sensor',
  lightSensorSub: 'Current light level',
  lightLuxLabel: 'Illuminance:',
  lightOn: 'On',
  lightOff: 'Off',
  lightSettingsSub: 'Display and sensitivity settings',
  lightShowGauge: 'Show light sensor gauge',
  lightThresholdLabel: '"On" threshold (lx):',
  lightThresholdHint: 'Light is considered on when sensor value exceeds this threshold.',
  feedJournal: 'Feed Journal',
  feedJournalSub: 'Last 20 feedings',
  feedJournalEmpty: 'No records',
  feedJournalSource: s => s === 'manual' ? 'Manual' : s === 'scheduled' ? 'Schedule' : s,

  schedule: 'Automatic Feeding',
  scheduleSub: 'Configure feeding schedule',
  nextFeedIn: (h, m) => `${h}h ${m}m`,
  nextFeedLabel: (h, m) => `Until next feed: ${h}h ${m}m`,
  repeats: 'Repeats:',
  day: 'Day:',
  addFeeding: '+ Add feeding',
  saveAllTimes: 'Save all times',
  days: ['Every day', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa', 'Su'],

  manualControl: 'Manual Control',
  manualControlSub: 'Manually control the servo',
  servoAngle: v => `Servo angle: ${v}°`,
  servoSpeed: v => `Servo speed: ${v}`,
  saveSpeed: 'Save speed',

  wifiInfo: 'WiFi Info',
  wifiInfoSub: 'Current network parameters',
  ssid: 'SSID:',
  ipAddr: 'IP Address:',
  modeLabel: 'Mode:',
  mdns: 'mDNS:',
  notConfigured: 'not configured',
  notConnected: 'not connected',
  apMode: 'Access Point (AP)',
  staMode: 'Station (STA)',

  batterySection: 'Battery',
  batterySectionSub: 'Device battery status',
  voltageValue: 'Voltage:',
  percentValue: 'Percent:',

  settingsInfo: 'Settings',
  settingsInfoSub: 'Current operating parameters',
  servoSpeedLabel: 'Servo speed:',
  feedRepeatsLabel: 'Feed repeats:',
  powerSaveLabel: 'Power save:',
  oledLabel: 'OLED display:',
  scheduleCountLabel: 'Schedule count:',
  on: 'Enabled',
  off: 'Disabled',

  systemInfo: 'System',
  systemInfoSub: 'Device information',
  model: 'Model:',
  firmwareVersion: 'Firmware version:',
  uptime: 'Uptime:',
  uptimeFmt: (h, m) => `${h}h ${m}m`,
  cpuFreq: 'CPU frequency:',
  deviceTime: 'Device time:',

  memoryInfo: 'Memory & Cache',
  memoryInfoSub: 'System resource usage',
  freeMemory: 'Free memory:',
  usedMemory: 'Used memory:',
  totalMemory: 'Total memory:',
  maxBlock: 'Max block:',
  minFree: 'Min free (ever):',
  cacheSizeLabel: 'Cache size:',
  cacheAgeLabel: 'Cache age:',
  cacheStatusLabel: 'Cache status:',
  cacheActive: '✅ Active',
  cacheInactive: '❌ Inactive',

  langTitle: 'Interface Language',
  langSub: 'Select web interface language',

  wifiSettings: 'WiFi Connection',
  wifiSettingsSub: 'Manage saved network',
  forgetNetwork: 'Forget Network',
  forgetHint: "The 'Forget' button removes saved network data and returns the device to access point mode. Connect to FishFeeder-XXXX and open 192.168.4.1 to reconfigure.",

  powerSettings: 'Power Settings',
  powerSettingsSub: 'Optimize power consumption',
  powerSaveToggle: 'Power saving mode',
  powerSaveHint: 'Power saving enabled: deep sleep after inactivity. Wake-up before schedule (timer) or by button. Web keeps device awake.',
  deepSleepLabel: 'Seconds idle before deep sleep (10–3600):',
  oledToggle: 'Enable OLED display',
  oledHint: 'Disable display to save energy. All functions work via web interface.',
  displayOffLabel: n => `Seconds to turn off OLED (power save, 5–600): Default ${n} s.`,
  savePowerSettings: 'Save power settings',

  feedInterval: 'Feed Interval',
  feedIntervalSub: 'Separate limit on feeding frequency',
  minIntervalLabel: 'Minimum interval between feedings (min, 1–1440):',
  minIntervalHint: n => `Limits how quickly a new feeding can start after the previous one. Default ${n} min.`,
  saveInterval: 'Save feed interval',

  calibration: 'Battery Calibration',
  calibrationSub: 'Manual voltage adjustment using a multimeter',
  calibrationCurrent: v => `Current sensor voltage: ${v} V`,
  calibrationLabel: 'Actual voltage from multimeter (V):',
  calibrationHint: 'Enter the multimeter value and apply. For preview, calibration is stored locally.',
  applyCalibration: 'Apply Calibration',
  calibrationInvalid: 'Enter a valid voltage (2.5–4.5 V)',

  timezone: 'Timezone',
  timezoneSub: 'Separately control device local time',
  timezoneLabel: 'Timezone (UTC offset):',
  timezoneHint: 'Select local UTC offset for the clock, schedule, and countdown to next feeding. Update manually when daylight saving time changes.',
  saveTimezone: 'Save timezone',

  account: 'Account',
  accountSub: 'Sign out of AquaFeed',
  signOut: 'Sign out',

  foodTitle: 'Food supply',
  foodRemaining: g => `${g} g remaining`,
  foodDuration: (m, d) => m > 0 ? `~${m} mo ${d} d` : `~${d} d`,
  foodRefill: 'Refill',
  foodGramsTotal: 'Loaded (g)',
  foodGramsPerFeed: 'Grams per feeding',
  foodSave: 'Save',
  foodCancel: 'Cancel',
  foodNotSet: 'Set food amount to track supply',
  foodNoSchedule: 'No active schedule',
  foodFeedingsPerDay: n => `${n} feedings/day`,

  tabStats: 'Statistics',
  statsFeedings: 'Feedings (last 7 days)',
  statsFeedingsToday: 'Feedings today',
  statsAvgPerDay: 'Avg / day',
  statsLight: 'Light duration (min)',
  statsLightToday: 'Light today',
  statsLightHours: (h, m) => h > 0 ? `${h}h ${m}m` : `${m}m`,
  statsRecommendations: 'Recommendations',
  recNoFeedingsToday: 'No feedings yet today — time to feed your fish!',
  recFeedMore: 'Feedings are infrequent. 2–4 times per day is recommended.',
  recFeedTooMuch: 'Too many feedings — excess food degrades water quality.',
  recNoLight: 'Light has not been on today — fish need a daily light cycle.',
  recLightTooShort: 'Light has been on for less than 8 hours. 8–12 hours per day is recommended.',
  recLightGood: 'Light duration is optimal. Keep it up!',
  recAllGood: 'All looks good! Happy fish.',
  dailyTips: [
    'Replace 20–30% of the water weekly to maintain balance and cleanliness.',
    'Check the filter once a month — a clogged filter degrades water quality.',
    'Don\'t overfeed — leftover food decomposes and raises ammonia levels.',
    'Keep water temperature stable: sudden changes stress your fish.',
    'Live plants absorb nitrates and oxygenate the water naturally.',
    'Clean the aquarium walls of algae once a week.',
    'After a long period without light, check your fish — they should be active.',
    'New fish need 2 weeks of quarantine before joining the main tank.',
    'Use a water conditioner to neutralize chlorine with every water change.',
    'Check pH regularly — most fish thrive in a 6.5–7.5 range.',
    'Proper lighting of 8–12 hours a day supports a natural cycle for fish and plants.',
    'Avoid placing the tank in direct sunlight — it causes algae blooms.',
    'Feed small portions 2–3 times a day rather than one large feeding.',
    'Nitrate levels above 40 mg/L are harmful — more frequent water changes help.',
    'Check the aerator monthly: good oxygenation is critical for fish health.',
  ],

  loginSignIn: 'Sign in',
  loginRegister: 'Register',
  loginEmail: 'Email',
  loginPassword: 'Password',
  loginSubtitle: 'Automatic fish feeder',
  loginNetworkError: 'Network error. Please try again.',
  loginLoading: 'Please wait…',

  toastFeeding: 'Feeding...',
  toastFed: 'Feeding started!',
  toastFeedError: 'Feeding error',
  toastScheduleSaved: 'Schedule saved',
  toastSaveError: 'Save error',
  toastSaved: 'Settings saved',
  toastFeedAdded: 'Feeding added',
  toastFeedRemoved: 'Feeding removed',
  toastPowerSave: on => `Power saving ${on ? 'enabled' : 'disabled'}`,
  toastError: 'Error',
  toastIntervalSaved: 'Interval saved',
  toastCalibDone: 'Calibration applied',
  toastCalibError: 'Calibration error',
  toastTimezoneSaved: 'Timezone saved',
  toastCommandSent: 'Command sent',
}

export const TRANSLATIONS: Record<Lang, Translations> = { uk, en }
