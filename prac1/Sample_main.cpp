#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

//#include <GL/glew.h>
#include <GLut/glut.h>
#include <algorithm>

#include <OpenGL/gl3.h>

using namespace std;

// 전역 변수 선언
GLuint programID;
GLuint VertexArrayID;
GLuint vertexbuffer;
GLint posAttrib;

// 점들을 저장할 벡터
vector<GLfloat> vertices;
bool isDrawing = false;

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
    
    //Compile Vertex Shader
    printf("Compiling shader : %s\n", file_path);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(ShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(ShaderID);
    
    //Check Vertex Shader
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

// 윈도우 좌표를 OpenGL 좌표로 변환하는 함수
void WindowToOpenGL(int x, int y, float *ox, float *oy) {
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    
    *ox = (2.0f * x / windowWidth) - 1.0f;
    *oy = 1.0f - (2.0f * y / windowHeight);
}

// 마우스 클릭 이벤트 처리 함수
void myMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // 마우스 좌표를 OpenGL 좌표로 변환
            float ox, oy;
            WindowToOpenGL(x, y, &ox, &oy);
            
            // 점 추가
            vertices.push_back(ox);
            vertices.push_back(oy);
            vertices.push_back(0.0f); // z 좌표는 0으로 설정
            
            // 버퍼 업데이트
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
            
            // 화면 갱신
            glutPostRedisplay();
        }
    }
}

// 렌더링 함수
void renderScene(void)
{
    // 화면 지우기
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 쉐이더 프로그램 사용
    glUseProgram(programID);
    
    // VAO 바인딩
    glBindVertexArray(VertexArrayID);
    
    // 버퍼 바인딩
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    
    // 쉐이더 attribute 활성화
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    // 점이 2개 이상 있을 때 라인 그리기
    if (vertices.size() >= 6) { // 최소 2개 점 (x, y, z 좌표 * 2)
        glDrawArrays(GL_LINES, 0, vertices.size() / 3);
    }
    
    // attribute 비활성화
    glDisableVertexAttribArray(posAttrib);
    
    // 버퍼 교체
    glutSwapBuffers();
}

// 초기화 함수
void init()
{
    // 배경색 설정
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    // VAO 생성
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    // VBO 생성
    glGenBuffers(1, &vertexbuffer);
    
    // 초기 점 데이터 설정 (샘플 직선)
    vertices.push_back(-0.5f);  // 첫 번째 점 x
    vertices.push_back(-0.5f);  // 첫 번째 점 y
    vertices.push_back(0.0f);   // 첫 번째 점 z
    
    vertices.push_back(0.5f);   // 두 번째 점 x
    vertices.push_back(0.5f);   // 두 번째 점 y
    vertices.push_back(0.0f);   // 두 번째 점 z
    
    // 버퍼에 데이터 입력
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
    
    // 쉐이더 로드
    programID = LoadShaders("VertexShader.txt", "FragmentShader.txt");
    glUseProgram(programID);
    
    // 쉐이더의 attribute 위치 얻기
    posAttrib = glGetAttribLocation(programID, "vtxPosition");
}

int main(int argc, char **argv)
{
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(480, 480);
    glutCreateWindow("Computer Graphics Assignment 1");

    // 초기화 함수 호출
    init();

    // 콜백 함수 등록
    glutDisplayFunc(renderScene);
    glutMouseFunc(myMouse); // 마우스 콜백 등록

    // GLUT 이벤트 루프 시작
    glutMainLoop();

    // 리소스 해제
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);
    
    return 1;
}
