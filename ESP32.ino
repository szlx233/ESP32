// 定义常量
const int sensorPins[] = {13, 12, 14, 27, 26}; // 传感器引脚
const int IN[2][2] = {{33, 32}, {18, 19}};     // 电机控制引脚
const int EN[] = {25, 23};                     // 电机 PWM 引脚
const int FORMS[][5] = {                       // 预定义巡线模式
  {1, 1, 0, 1, 1}, // 中间
  {1, 1, 1, 0, 1}, // 右调整
  {1, 0, 1, 1, 1}, // 左调整
  {0, 0, 0, 0, 0}, // 交叉
  {1, 1, 0, 0, 0}, // 交叉延伸 - T1
  {0, 0, 0, 1, 1}, // 交叉延伸 - T2
  {1, 1, 1, 0, 0}, // 交叉延伸 - T3
  {0, 0, 1, 1, 1},  // 交叉延伸 - T4
  {1, 1, 1, 1, 0}, // 交叉延伸 - T5
  {0, 1, 1, 1, 1}, // 交叉延伸 - T6
};

// 全局变量
int Data[5] = {0};
int x_y[2] = {0, 0}; // 当前位置
int face_to = 1;     // 当前朝向
// int chessboard[5][5] = { // 棋盘
//   {2, 2 ,2, 2, 2},
//   {2, 2 ,2, 2, 2},
//   {2, 2 ,2, 2, 2},
//   {2, 2 ,2, 2, 2},
// };

int chessboard[5][5] = { // 棋盘
  {2, 2, -1, -1, -1},
  {-1, 1, 2, -1, -1},
  {2, -1, 2, -1, -1},
  {-1, -1, -1, -1, -1},
};

// 初始化
void setup() {
  Serial.begin(9600);

  delay(1000);
  // 初始化引脚
  for (int pin : sensorPins) pinMode(pin, INPUT);
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) pinMode(IN[i][j], OUTPUT);
    pinMode(EN[i], OUTPUT);
  }
}

// 主循环
void loop() {
  getData(); // 更新传感器数据

  if (matches(FORMS[0])) {
    Serial.println("Center!");
    delay(1);
    moveDifferential(150, 150); // 差速前进
  } else if (matches(FORMS[1])) {
    Serial.println("Right adjust!");
    moveDifferential(50, 150); // 右侧调整
  } else if (matches(FORMS[2])) {
    Serial.println("Left adjust!");
    moveDifferential(150, 50); // 左侧调整
  } else if (isCrossRoad()) {
    Serial.println("Crossroad!");
    handleCrossRoad();
  }
}

// 获取传感器数据
void getData() {
  for (int i = 0; i < 5; i++) Data[i] = digitalRead(sensorPins[i]);
}

// 判断当前数据是否匹配某种模式
bool matches(const int form[]) {
  for (int i = 0; i < 5; i++) if (Data[i] != form[i]) return false;
  return true;
}

// 检查是否处于交叉点
bool isCrossRoad() {
  for (int i = 3; i < 10; i++) if (matches(FORMS[i])) return true;
  return false;
}

// 差速控制移动
void moveDifferential(int speedLeft, int speedRight) {
  // 左电机设置
  if (speedLeft > 0) {
    digitalWrite(IN[0][0], HIGH);
    digitalWrite(IN[0][1], LOW);
  } else {
    digitalWrite(IN[0][0], LOW);
    digitalWrite(IN[0][1], HIGH);
    speedLeft = -speedLeft;
  }

  // 右电机设置
  if (speedRight > 0) {
    digitalWrite(IN[1][0], HIGH);
    digitalWrite(IN[1][1], LOW);
  } else {
    digitalWrite(IN[1][0], LOW);
    digitalWrite(IN[1][1], HIGH);
    speedRight = -speedRight;
  }

  // 设置PWM
  analogWrite(EN[0], speedLeft);
  analogWrite(EN[1], speedRight);
}

// 停止
void stop() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(IN[i][0], LOW);
    digitalWrite(IN[i][1], LOW);
  }
  analogWrite(EN[0], 0);
  analogWrite(EN[1], 0);
}

// 原地旋转
void rotate(int direction) {
  if (direction == 1) { // 左转
    digitalWrite(IN[0][0], LOW);
    digitalWrite(IN[0][1], HIGH);
    digitalWrite(IN[1][0], HIGH);
    digitalWrite(IN[1][1], LOW);
  } else if (direction == 2) { // 右转
    digitalWrite(IN[0][0], HIGH);
    digitalWrite(IN[0][1], LOW);
    digitalWrite(IN[1][0], LOW);
    digitalWrite(IN[1][1], HIGH);
  }
  analogWrite(EN[0], 150);
  analogWrite(EN[1], 150);
}

// 处理交叉路口逻辑
void handleCrossRoad() {
  updatePosition();
  int action = chessboard[x_y[0]][x_y[1]];
  
  Serial.print("At crossroad, x_y=(");
  Serial.print(x_y[0]);
  Serial.print(", ");
  Serial.print(x_y[1]);
  Serial.print("), action=");
  Serial.println(action);
  Serial.println(face_to);


  switch (action) {
    case 0:
      Serial.println("Moving forward");
      moveDifferential(150, 150);
      delay(500);
      break;
    case 1:
      Serial.println("Turning left");
      delay(440);
      rotateAndAlign(1);
      break;
    case 2:
      Serial.println("Turning right");
      delay(440);
      rotateAndAlign(2);
      break;
    case 3:
    case -1:
      Serial.println("Stopping");
      moveDifferential(150, 150);
      delay(320);
      stop();
      break;
  }
}

// 更新当前位置
void updatePosition() {
  switch (face_to) {
    case 1: x_y[1]++; break;
    case 2: x_y[0]++; break;
    case 3: x_y[1]--; break;
    case 4: x_y[0]--; break;
  }
}

// 转弯并对齐
void rotateAndAlign(int direction) {
  rotate(direction);
  while (!matches(FORMS[0])) getData(); // 等待对齐
  // delay(10);
  if (direction == 1) {  // 左转
    face_to = face_to - 1;  
    if (face_to < 1) {   // 如果小车的朝向低于1，循环到4
        face_to = 4;
    }
  } else if (direction == 2) {  // 右转
      face_to = face_to + 1;
      if (face_to > 4) {   // 如果小车的朝向大于4，循环到1
          face_to = 1;
      }
  }
 // 更新朝向
}
