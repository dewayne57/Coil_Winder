#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

constexpr uint8_t LCD_RS = 22;
constexpr uint8_t LCD_EN = 23;
constexpr uint8_t LCD_D4 = 24;
constexpr uint8_t LCD_D5 = 25;
constexpr uint8_t LCD_D6 = 26;
constexpr uint8_t LCD_D7 = 27;

constexpr uint8_t ROW_PINS[] = {30, 31, 32, 33};
constexpr uint8_t COL_PINS[] = {34, 35, 36, 37};

constexpr char KEYMAP[4][4] = {
    {'1', '2', '3', 'U'},
    {'4', '5', '6', 'D'},
    {'7', '8', '9', 'Y'},
    {'.', '0', 'N', 'E'},
};

constexpr uint8_t LCD_COLUMNS = 20;
constexpr uint8_t LCD_ROWS = 4;
constexpr size_t INPUT_BUFFER_SIZE = 16;
constexpr unsigned long DEBOUNCE_MS = 35;
constexpr unsigned long DISPLAY_REFRESH_MS = 200;

class SimpleLcd {
 public:
  SimpleLcd(uint8_t rs, uint8_t enable, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
      : rsPin(rs), enablePin(enable), dataPins{d4, d5, d6, d7} {}

  void begin(uint8_t columns, uint8_t rows) {
    lcdColumns = columns;
    lcdRows = rows;

    pinMode(rsPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    for (uint8_t dataPin : dataPins) {
      pinMode(dataPin, OUTPUT);
    }

    delay(50);
    digitalWrite(rsPin, LOW);
    writeNibble(0x03);
    delay(5);
    writeNibble(0x03);
    delayMicroseconds(150);
    writeNibble(0x03);
    writeNibble(0x02);

    command(0x28);
    command(0x08);
    command(0x01);
    delay(2);
    command(0x06);
    command(0x0C);
  }

  void setCursor(uint8_t column, uint8_t row) {
    static const uint8_t ROW_OFFSETS[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= lcdRows) {
      row = lcdRows - 1;
    }
    if (column >= lcdColumns) {
      column = lcdColumns - 1;
    }
    command(static_cast<uint8_t>(0x80 | (column + ROW_OFFSETS[row])));
  }

  void print(const char *text) {
    while (*text != '\0') {
      write(static_cast<uint8_t>(*text++), true);
    }
  }

 private:
  uint8_t rsPin;
  uint8_t enablePin;
  uint8_t dataPins[4];
  uint8_t lcdColumns = 0;
  uint8_t lcdRows = 0;

  void pulseEnable() {
    digitalWrite(enablePin, LOW);
    delayMicroseconds(1);
    digitalWrite(enablePin, HIGH);
    delayMicroseconds(1);
    digitalWrite(enablePin, LOW);
    delayMicroseconds(100);
  }

  void writeNibble(uint8_t value) {
    for (uint8_t bit = 0; bit < 4; ++bit) {
      digitalWrite(dataPins[bit], (value >> bit) & 0x01);
    }
    pulseEnable();
  }

  void write(uint8_t value, bool dataMode) {
    digitalWrite(rsPin, dataMode ? HIGH : LOW);
    writeNibble(static_cast<uint8_t>(value >> 4));
    writeNibble(static_cast<uint8_t>(value & 0x0F));
  }

  void command(uint8_t value) {
    write(value, false);
  }
};

SimpleLcd lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

enum FormShape : uint8_t {
  FORM_ROUND = 0,
  FORM_RECTANGULAR,
  FORM_OVAL,
  FORM_SHAPE_COUNT
};

enum TargetMode : uint8_t {
  TARGET_TURNS = 0,
  TARGET_INDUCTANCE,
  TARGET_MODE_COUNT
};

enum WindingMode : uint8_t {
  WINDING_SINGLE = 0,
  WINDING_BIFILAR,
  WINDING_MULTI,
  WINDING_MODE_COUNT
};

enum MenuItem : uint8_t {
  ITEM_WIRE_SIZE = 0,
  ITEM_FORM_SHAPE,
  ITEM_ROUND_DIAMETER,
  ITEM_FORM_WIDTH,
  ITEM_FORM_HEIGHT,
  ITEM_TARGET_MODE,
  ITEM_TURNS,
  ITEM_INDUCTANCE,
  ITEM_LAYERS,
  ITEM_WINDING_MODE,
  ITEM_FILAR_COUNT,
  ITEM_READY_SCREEN,
  ITEM_COUNT
};

struct CoilSettings {
  float wireSizeMm = 0.25f;
  FormShape formShape = FORM_ROUND;
  float roundDiameterMm = 12.0f;
  float formWidthMm = 20.0f;
  float formHeightMm = 10.0f;
  TargetMode targetMode = TARGET_TURNS;
  unsigned long turns = 100;
  float inductanceMilliHenry = 1.0f;
  uint16_t layers = 1;
  WindingMode windingMode = WINDING_SINGLE;
  uint8_t filarCount = 1;
};

CoilSettings settings;
MenuItem currentItem = ITEM_WIRE_SIZE;
bool editingValue = false;
bool displayDirty = true;
char inputBuffer[INPUT_BUFFER_SIZE] = "";
char machineStatus[21] = "Status: Configuring";
unsigned long lastDisplayRefresh = 0;
char lastRawKey = '\0';
char stableKey = '\0';
unsigned long lastKeyChangeAt = 0;

const char *const FORM_NAMES[] = {"Round", "Rectangular", "Oval"};
const char *const TARGET_MODE_NAMES[] = {"Turns", "Inductance"};
const char *const WINDING_MODE_NAMES[] = {"Single", "Bifilar", "Multi-filar"};

bool isNumericItem(MenuItem item) {
  return item == ITEM_WIRE_SIZE || item == ITEM_ROUND_DIAMETER ||
         item == ITEM_FORM_WIDTH || item == ITEM_FORM_HEIGHT ||
         item == ITEM_TURNS || item == ITEM_INDUCTANCE ||
         item == ITEM_LAYERS || item == ITEM_FILAR_COUNT;
}

bool isChoiceItem(MenuItem item) {
  return item == ITEM_FORM_SHAPE || item == ITEM_TARGET_MODE ||
         item == ITEM_WINDING_MODE;
}

bool isItemVisible(MenuItem item) {
  switch (item) {
    case ITEM_ROUND_DIAMETER:
      return settings.formShape == FORM_ROUND;
    case ITEM_FORM_WIDTH:
    case ITEM_FORM_HEIGHT:
      return settings.formShape != FORM_ROUND;
    case ITEM_TURNS:
      return settings.targetMode == TARGET_TURNS;
    case ITEM_INDUCTANCE:
      return settings.targetMode == TARGET_INDUCTANCE;
    case ITEM_FILAR_COUNT:
      return settings.windingMode == WINDING_MULTI;
    default:
      return true;
  }
}

MenuItem nextVisibleItem(MenuItem item, int direction) {
  int index = static_cast<int>(item);
  do {
    index += direction;
    if (index < 0) {
      index = ITEM_COUNT - 1;
    } else if (index >= ITEM_COUNT) {
      index = 0;
    }
  } while (!isItemVisible(static_cast<MenuItem>(index)));

  return static_cast<MenuItem>(index);
}

void setStatus(const char *status) {
  strncpy(machineStatus, status, sizeof(machineStatus) - 1);
  machineStatus[sizeof(machineStatus) - 1] = '\0';
  displayDirty = true;
}

void printLine(uint8_t row, const char *text) {
  char buffer[LCD_COLUMNS + 1];
  snprintf(buffer, sizeof(buffer), "%-20.20s", text);
  lcd.setCursor(0, row);
  lcd.print(buffer);
}

void formatFloatValue(float value, const char *suffix, char *buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%.2f %s", value, suffix);
}

void formatIntegerValue(unsigned long value, const char *suffix, char *buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%lu %s", value, suffix);
}

void syncFilarCount() {
  if (settings.windingMode == WINDING_SINGLE) {
    settings.filarCount = 1;
  } else if (settings.windingMode == WINDING_BIFILAR) {
    settings.filarCount = 2;
  } else if (settings.filarCount < 3) {
    settings.filarCount = 3;
  }
}

void formatValue(MenuItem item, char *buffer, size_t bufferSize) {
  switch (item) {
    case ITEM_WIRE_SIZE:
      formatFloatValue(settings.wireSizeMm, "mm", buffer, bufferSize);
      break;
    case ITEM_FORM_SHAPE:
      snprintf(buffer, bufferSize, "%s", FORM_NAMES[settings.formShape]);
      break;
    case ITEM_ROUND_DIAMETER:
      formatFloatValue(settings.roundDiameterMm, "mm", buffer, bufferSize);
      break;
    case ITEM_FORM_WIDTH:
      formatFloatValue(settings.formWidthMm, "mm", buffer, bufferSize);
      break;
    case ITEM_FORM_HEIGHT:
      formatFloatValue(settings.formHeightMm, "mm", buffer, bufferSize);
      break;
    case ITEM_TARGET_MODE:
      snprintf(buffer, bufferSize, "%s", TARGET_MODE_NAMES[settings.targetMode]);
      break;
    case ITEM_TURNS:
      formatIntegerValue(settings.turns, "turns", buffer, bufferSize);
      break;
    case ITEM_INDUCTANCE:
      formatFloatValue(settings.inductanceMilliHenry, "mH", buffer, bufferSize);
      break;
    case ITEM_LAYERS:
      formatIntegerValue(settings.layers, "layers", buffer, bufferSize);
      break;
    case ITEM_WINDING_MODE:
      snprintf(buffer, bufferSize, "%s", WINDING_MODE_NAMES[settings.windingMode]);
      break;
    case ITEM_FILAR_COUNT:
      formatIntegerValue(settings.filarCount, "wires", buffer, bufferSize);
      break;
    case ITEM_READY_SCREEN:
      snprintf(buffer, bufferSize, "Y arm / N reset");
      break;
    default:
      buffer[0] = '\0';
      break;
  }
}

const char *labelFor(MenuItem item) {
  switch (item) {
    case ITEM_WIRE_SIZE:
      return "Wire size";
    case ITEM_FORM_SHAPE:
      return "Form shape";
    case ITEM_ROUND_DIAMETER:
      return "Form diameter";
    case ITEM_FORM_WIDTH:
      return "Form width";
    case ITEM_FORM_HEIGHT:
      return "Form height";
    case ITEM_TARGET_MODE:
      return "Target mode";
    case ITEM_TURNS:
      return "Target turns";
    case ITEM_INDUCTANCE:
      return "Target inductance";
    case ITEM_LAYERS:
      return "Layers";
    case ITEM_WINDING_MODE:
      return "Winding mode";
    case ITEM_FILAR_COUNT:
      return "Filar count";
    case ITEM_READY_SCREEN:
      return "Program review";
    default:
      return "";
  }
}

void seedInputBuffer(MenuItem item) {
  char currentValue[INPUT_BUFFER_SIZE];

  switch (item) {
    case ITEM_WIRE_SIZE:
      snprintf(currentValue, sizeof(currentValue), "%.2f", settings.wireSizeMm);
      break;
    case ITEM_ROUND_DIAMETER:
      snprintf(currentValue, sizeof(currentValue), "%.2f", settings.roundDiameterMm);
      break;
    case ITEM_FORM_WIDTH:
      snprintf(currentValue, sizeof(currentValue), "%.2f", settings.formWidthMm);
      break;
    case ITEM_FORM_HEIGHT:
      snprintf(currentValue, sizeof(currentValue), "%.2f", settings.formHeightMm);
      break;
    case ITEM_TURNS:
      snprintf(currentValue, sizeof(currentValue), "%lu", settings.turns);
      break;
    case ITEM_INDUCTANCE:
      snprintf(currentValue, sizeof(currentValue), "%.2f", settings.inductanceMilliHenry);
      break;
    case ITEM_LAYERS:
      snprintf(currentValue, sizeof(currentValue), "%u", settings.layers);
      break;
    case ITEM_FILAR_COUNT:
      snprintf(currentValue, sizeof(currentValue), "%u", settings.filarCount);
      break;
    default:
      currentValue[0] = '\0';
      break;
  }

  strncpy(inputBuffer, currentValue, sizeof(inputBuffer) - 1);
  inputBuffer[sizeof(inputBuffer) - 1] = '\0';
}

bool applyNumericInput(MenuItem item) {
  if (inputBuffer[0] == '\0') {
    setStatus("Status: Input needed");
    return false;
  }

  const float floatValue = static_cast<float>(atof(inputBuffer));
  const unsigned long unsignedValue = strtoul(inputBuffer, nullptr, 10);

  switch (item) {
    case ITEM_WIRE_SIZE:
      if (floatValue <= 0.0f) {
        setStatus("Status: Wire > 0");
        return false;
      }
      settings.wireSizeMm = floatValue;
      break;
    case ITEM_ROUND_DIAMETER:
      if (floatValue <= 0.0f) {
        setStatus("Status: Dia > 0");
        return false;
      }
      settings.roundDiameterMm = floatValue;
      break;
    case ITEM_FORM_WIDTH:
      if (floatValue <= 0.0f) {
        setStatus("Status: Width > 0");
        return false;
      }
      settings.formWidthMm = floatValue;
      break;
    case ITEM_FORM_HEIGHT:
      if (floatValue <= 0.0f) {
        setStatus("Status: Height > 0");
        return false;
      }
      settings.formHeightMm = floatValue;
      break;
    case ITEM_TURNS:
      if (unsignedValue == 0) {
        setStatus("Status: Turns > 0");
        return false;
      }
      settings.turns = unsignedValue;
      break;
    case ITEM_INDUCTANCE:
      if (floatValue <= 0.0f) {
        setStatus("Status: Induct > 0");
        return false;
      }
      settings.inductanceMilliHenry = floatValue;
      break;
    case ITEM_LAYERS:
      if (unsignedValue == 0 || unsignedValue > 999) {
        setStatus("Status: 1-999 layers");
        return false;
      }
      settings.layers = static_cast<uint16_t>(unsignedValue);
      break;
    case ITEM_FILAR_COUNT:
      if (unsignedValue < 3 || unsignedValue > 9) {
        setStatus("Status: 3-9 wires");
        return false;
      }
      settings.filarCount = static_cast<uint8_t>(unsignedValue);
      break;
    default:
      return false;
  }

  syncFilarCount();
  setStatus("Status: Value saved");
  return true;
}

void cycleChoice(MenuItem item, int direction) {
  switch (item) {
    case ITEM_FORM_SHAPE:
      settings.formShape = static_cast<FormShape>((settings.formShape + FORM_SHAPE_COUNT + direction) % FORM_SHAPE_COUNT);
      setStatus("Status: Shape set");
      break;
    case ITEM_TARGET_MODE:
      settings.targetMode = static_cast<TargetMode>((settings.targetMode + TARGET_MODE_COUNT + direction) % TARGET_MODE_COUNT);
      setStatus("Status: Target set");
      break;
    case ITEM_WINDING_MODE:
      settings.windingMode = static_cast<WindingMode>((settings.windingMode + WINDING_MODE_COUNT + direction) % WINDING_MODE_COUNT);
      syncFilarCount();
      setStatus("Status: Winding set");
      break;
    default:
      break;
  }

  if (!isItemVisible(currentItem)) {
    currentItem = nextVisibleItem(currentItem, 1);
  }
  displayDirty = true;
}

void beginNumericEdit(MenuItem item) {
  editingValue = true;
  seedInputBuffer(item);
  setStatus("Status: Edit digits");
}

void cancelNumericEdit() {
  editingValue = false;
  inputBuffer[0] = '\0';
  setStatus("Status: Edit canceled");
}

void commitNumericEdit() {
  if (applyNumericInput(currentItem)) {
    editingValue = false;
    inputBuffer[0] = '\0';
  }
}

void appendInputCharacter(char key) {
  const size_t length = strlen(inputBuffer);
  if (length >= INPUT_BUFFER_SIZE - 1) {
    setStatus("Status: Input full");
    return;
  }

  if (key == '.' && strchr(inputBuffer, '.') != nullptr) {
    setStatus("Status: One decimal");
    return;
  }

  inputBuffer[length] = key;
  inputBuffer[length + 1] = '\0';
  setStatus("Status: Editing");
}

void backspaceInput() {
  const size_t length = strlen(inputBuffer);
  if (length == 0) {
    cancelNumericEdit();
    return;
  }

  inputBuffer[length - 1] = '\0';
  setStatus("Status: Editing");
}

char scanRawKey() {
  for (uint8_t row = 0; row < 4; ++row) {
    digitalWrite(ROW_PINS[row], LOW);
    delayMicroseconds(5);

    for (uint8_t col = 0; col < 4; ++col) {
      if (digitalRead(COL_PINS[col]) == LOW) {
        digitalWrite(ROW_PINS[row], HIGH);
        return KEYMAP[row][col];
      }
    }

    digitalWrite(ROW_PINS[row], HIGH);
  }

  return '\0';
}

char readKey() {
  const char rawKey = scanRawKey();
  const unsigned long now = millis();

  if (rawKey != lastRawKey) {
    lastRawKey = rawKey;
    lastKeyChangeAt = now;
  }

  if ((now - lastKeyChangeAt) >= DEBOUNCE_MS && rawKey != stableKey) {
    stableKey = rawKey;
    if (stableKey != '\0') {
      return stableKey;
    }
  }

  return '\0';
}

void renderReadySummary() {
  char line2[21];
  snprintf(line2, sizeof(line2), "%s %uL %s",
           settings.targetMode == TARGET_TURNS ? "Turns" : "Induct",
           settings.layers,
           settings.windingMode == WINDING_MULTI ? "Multi" : WINDING_MODE_NAMES[settings.windingMode]);
  printLine(2, line2);
}

void renderMenu() {
  char valueLine[21];
  char helpLine[21];

  printLine(0, machineStatus);
  printLine(1, labelFor(currentItem));

  if (currentItem == ITEM_READY_SCREEN) {
    renderReadySummary();
    printLine(3, "Y=Arm   N=Reset");
    return;
  }

  if (editingValue) {
    snprintf(valueLine, sizeof(valueLine), "> %s", inputBuffer);
    printLine(2, valueLine);
    printLine(3, "N=Erase E=Save");
    return;
  }

  formatValue(currentItem, valueLine, sizeof(valueLine));
  printLine(2, valueLine);

  if (isChoiceItem(currentItem)) {
    snprintf(helpLine, sizeof(helpLine), "Y/N change  U/D nav");
  } else {
    snprintf(helpLine, sizeof(helpLine), "E edit    U/D nav");
  }

  printLine(3, helpLine);
}

void handleReadyKey(char key) {
  if (key == 'Y') {
    setStatus("Status: Program armed");
  } else if (key == 'N') {
    settings = CoilSettings();
    currentItem = ITEM_WIRE_SIZE;
    editingValue = false;
    inputBuffer[0] = '\0';
    syncFilarCount();
    setStatus("Status: Defaults loaded");
  }
}

void handleBrowsingKey(char key) {
  if (key == 'U') {
    currentItem = nextVisibleItem(currentItem, -1);
    setStatus("Status: Browse");
    return;
  }

  if (key == 'D') {
    currentItem = nextVisibleItem(currentItem, 1);
    setStatus("Status: Browse");
    return;
  }

  if (currentItem == ITEM_READY_SCREEN) {
    handleReadyKey(key);
    return;
  }

  if (isChoiceItem(currentItem)) {
    if (key == 'Y' || key == 'E') {
      cycleChoice(currentItem, 1);
    } else if (key == 'N') {
      cycleChoice(currentItem, -1);
    }
    return;
  }

  if (isNumericItem(currentItem)) {
    if (key == 'E') {
      beginNumericEdit(currentItem);
    } else if (isdigit(key) || key == '.') {
      editingValue = true;
      inputBuffer[0] = '\0';
      appendInputCharacter(key);
    }
  }
}

void handleEditingKey(char key) {
  if (isdigit(key) || key == '.') {
    appendInputCharacter(key);
  } else if (key == 'N') {
    backspaceInput();
  } else if (key == 'E' || key == 'Y') {
    commitNumericEdit();
  } else if (key == 'D' || key == 'U') {
    setStatus("Status: Save first");
  }
}

void setupKeypad() {
  for (uint8_t rowPin : ROW_PINS) {
    pinMode(rowPin, OUTPUT);
    digitalWrite(rowPin, HIGH);
  }

  for (uint8_t colPin : COL_PINS) {
    pinMode(colPin, INPUT_PULLUP);
  }
}

}  // namespace

void setup() {
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  setupKeypad();
  syncFilarCount();
  renderMenu();
}

void loop() {
  const char key = readKey();
  if (key != '\0') {
    if (editingValue) {
      handleEditingKey(key);
    } else {
      handleBrowsingKey(key);
    }
    displayDirty = true;
  }

  const unsigned long now = millis();
  if (displayDirty || (now - lastDisplayRefresh) >= DISPLAY_REFRESH_MS) {
    renderMenu();
    displayDirty = false;
    lastDisplayRefresh = now;
  }
}
