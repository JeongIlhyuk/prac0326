#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdlib>

//#include <GL/glew.h>
#include <GLut/glut.h>
#include <algorithm>

#include <OpenGL/gl3.h>

using namespace std;

GLuint programID;
GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint colorbuffer;  // 색상 버퍼
GLint posAttrib;
GLint colorAttrib;   // 색상 어트리뷰트
struct Point {
    float x, y, z;
    float r, g, b, a;
};

vector<Point> points;

// 드래그
bool isDragging = false;
float dragStartX, dragStartY;
float dragEndX, dragEndY;

// 랜덤 색상
void getRandomColor(float &r, float &g, float &b) {
    r = (float)rand() / RAND_MAX;
    g = (float)rand() / RAND_MAX;
    b = (float)rand() / RAND_MAX;
}

static void CompileShader(int &InfoLogLength, GLint &Result, GLuint ShaderID, const char *file_path) {
    string VertexShaderCode;
    ifstream VertexShaderStream(file_path, ios::in);
    if(VertexShaderStream.is_open())
    {
        string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    
    //Compile Shader
    printf("Compiling shader : %s\n", file_path);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(ShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(ShaderID);
    
    //Check Shader
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if(InfoLogLength !=0){
        vector<char> VertexShaderErrorMessage(InfoLogLength);
        glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    }
}

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    //create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    //Read the vertex shader code from the file
    CompileShader(InfoLogLength, Result, VertexShaderID, vertex_file_path);
    CompileShader(InfoLogLength, Result, FragmentShaderID, fragment_file_path);

    //Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
 
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if(InfoLogLength !=0){
        vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
    }
 
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
 
    return ProgramID;
}

void WindowToOpenGL(int x, int y, float *ox, float *oy) {
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    
    *ox = (2.0f * x / windowWidth) - 1.0f;
    *oy = 1.0f - (2.0f * y / windowHeight);
}

void updateBuffers() {
    // 위치 버퍼 데이터 생성
    vector<GLfloat> vertices;
    for(size_t i = 0; i < points.size(); i++) {
        vertices.push_back(points[i].x);
        vertices.push_back(points[i].y);
        vertices.push_back(points[i].z);
    }
    
    // 색상 버퍼 데이터 생성
    vector<GLfloat> colors;
    for(size_t i = 0; i < points.size(); i++) {
        colors.push_back(points[i].r);
        colors.push_back(points[i].g);
        colors.push_back(points[i].b);
        colors.push_back(points[i].a);
    }
    
    // 버퍼 업데이트
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), &colors[0], GL_STATIC_DRAW);
}

// 점이 사각형 안에 있는지 확인
bool isPointInRect(float x, float y, float x1, float y1, float x2, float y2) {
    float minX = min(x1, x2);
    float maxX = max(x1, x2);
    float minY = min(y1, y2);
    float maxY = max(y1, y2);
    
    return (x >= minX && x <= maxX && y >= minY && y <= maxY);
}

void myMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        float ox, oy;
        WindowToOpenGL(x, y, &ox, &oy);
        
        if (state == GLUT_DOWN) {
            // 드래그 시작
            isDragging = true;
            dragStartX = ox;
            dragStartY = oy;
            
            Point newPoint;
            newPoint.x = ox;
            newPoint.y = oy;
            newPoint.z = 0.0f;
            
            getRandomColor(newPoint.r, newPoint.g, newPoint.b);
            newPoint.a = 1.0f;
            
            points.push_back(newPoint);
            updateBuffers();
            
            // 화면 갱신
            glutPostRedisplay();
        }
        else if (state == GLUT_UP && isDragging) {
            // 드래그 끝
            isDragging = false;
            dragEndX = ox;
            dragEndY = oy;
            
            // 선택된 영역 안의 점들 색상 변경
            float r, g, b;
            getRandomColor(r, g, b);
            
            for(size_t i = 0; i < points.size(); i++) {
                if(isPointInRect(points[i].x, points[i].y, dragStartX, dragStartY, dragEndX, dragEndY)) {
                    points[i].r = r;
                    points[i].g = g;
                    points[i].b = b;
                }
            }
            
            updateBuffers();
            glutPostRedisplay();
        }
    }
}

// 마우스 이동 이벤트
void myMotion(int x, int y) {
    if (isDragging) {
        float ox, oy;
        WindowToOpenGL(x, y, &ox, &oy);
        dragEndX = ox;
        dragEndY = oy;
        glutPostRedisplay();
    }
}

void drawDragRect() {
    if (isDragging) {
        glUseProgram(0);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glColor4f(0.5f, 0.5f, 1.0f, 0.3f);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glBegin(GL_QUADS);
        glVertex2f(dragStartX, dragStartY);
        glVertex2f(dragEndX, dragStartY);
        glVertex2f(dragEndX, dragEndY);
        glVertex2f(dragStartX, dragEndY);
        glEnd();
        
        glDisable(GL_BLEND);
    }
}

// 렌더링
void renderScene(void)
{
    // 화면 지우기
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 셰이더 프로그램 사용
    glUseProgram(programID);
    
    // VAO 바인딩
    glBindVertexArray(VertexArrayID);
    
    // 포지션 버퍼 바인딩
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    // 색상 버퍼 바인딩
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    // 점 그리기
    if(points.size() > 0) {
        glDrawArrays(GL_POINTS, 0, points.size());
    }
    
    // attribute 비활성화
    glDisableVertexAttribArray(posAttrib);
    glDisableVertexAttribArray(colorAttrib);
    
    // 드래그 사각형 그리기
    drawDragRect();
    
    glutSwapBuffers();
}

// 초기화 함수
void init()
{
    srand(time(NULL));
    
    // 배경색 설정
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    // VAO 생성
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    // VBO 생성
    glGenBuffers(1, &vertexbuffer);
    glGenBuffers(1, &colorbuffer);  // 색상 버퍼 생성
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(480, 480);
    glutCreateWindow("Random Color Points");

    init();
    
    programID = LoadShaders("VertexShader.txt", "FragmentShader.txt");
    glUseProgram(programID);
    
    // 어트리뷰트 위치 가져오기
    posAttrib = glGetAttribLocation(programID, "vtxPosition");
    colorAttrib = glGetAttribLocation(programID, "vtxColor");

    // 콜백 함수 등록
    glutDisplayFunc(renderScene);
    glutMouseFunc(myMouse);     // 마우스 클릭 콜백
    glutMotionFunc(myMotion);   // 마우스 이동 콜백

    // GLUT 이벤트 루프 시작
    glutMainLoop();

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);
    
    return 1;
}
