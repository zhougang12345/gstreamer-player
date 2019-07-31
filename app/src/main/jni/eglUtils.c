//
// Created by zhougang on 2019/5/24.
//


#include "eglUtils.h"
const char * vertexShaderCode = "attribute vec4 aPosition;\n"
                                 "attribute vec2 fPosition;\n"
                                 "varying vec2 tPosition;\n"
                                 "void main() \n"
                                 "{\n"
                                 "    tPosition = fPosition;\n"
                                 "    gl_Position = aPosition;\n"
                                 "}";
const char * fragmentShaderCode0 ="precision mediump float;\n"
                                   "varying vec2 tPosition;\n"
                                   "uniform sampler2D sTexture;\n"
                                   "void main()\n"
                                   "{\n"
                                   "    gl_FragColor = texture2D(sTexture,tPosition);\n"
                                   "}";
const char * fragmentShaderCode = "#extension GL_OES_EGL_image_external : require\n"
                                   "precision mediump float;\n"
                                   "varying vec2 tPosition;\n"
                                   "uniform samplerExternalOES sTexture;\n"
                                   "void main()\n"
                                   "{\n"
                                   "    gl_FragColor = texture2D(sTexture,tPosition);\n"
                                   "}";
GLfloat vertexArray[8] = {
        -1.0f,-1.0f,
        1.0f,-1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
};
GLfloat fragArray[8] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
};

void draw(RENDER *render,GLuint textureId,int type){

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    glUseProgram(render->program);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    if (type == sample2D){
        glBindTexture(GL_TEXTURE_2D,textureId);
    }else {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
    }
    glUniform1i(render->sTexture,0);
    glEnableVertexAttribArray(render->aPositionHandle);
    glVertexAttribPointer(render->aPositionHandle, 2, GL_FLOAT, GL_FALSE, 2 * 4, vertexArray);
    glEnableVertexAttribArray(render->fPositionHandle);
    glVertexAttribPointer(render->fPositionHandle, 2, GL_FLOAT, GL_FALSE, 2 * 4, fragArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D,0);
    glDisable(GL_TEXTURE_2D);
}
static int compileShader(int type, const char * shaderCode) {

    int shader = glCreateShader(type);
    if (shader == 0) {
        return 0;
    }
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);
    GLint compileStatus = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static int createProgram(RENDER *render,const char *vertexShaderCode,const char *fragmentShaderCode) {
    GLint program = glCreateProgram();
    if (0 == program) {
        return 0;
    }

    render->vertexShaderID = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
    render->fragmentShaderID = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);

    glAttachShader(program, render->vertexShaderID);
    glAttachShader(program, render->fragmentShaderID);
    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (0 == linkStatus) {
        glDeleteProgram(program);
        return 0;
    }
    return program;

}
void openRenderProg(RENDER *render,int type) {
    if (type == sample2D) {
        render->program = createProgram(render, vertexShaderCode, fragmentShaderCode0);
    } else {
        render->program = createProgram(render, vertexShaderCode, fragmentShaderCode);
    }

    render->aPositionHandle = glGetAttribLocation(render->program, "aPosition");
    render->fPositionHandle = glGetAttribLocation(render->program, "fPosition");
    render->sTexture = glGetUniformLocation(render->program, "sTexture");
}
void genTexture(GLuint *textureId){
    glGenTextures(1,textureId);
    glBindTexture(GL_TEXTURE_2D,*textureId);
    glActiveTexture(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,0);
}
void blindTexture(GLuint textureId,void* texture,GLsizei texWidth,GLsizei texHeight){
    glBindTexture(GL_TEXTURE_2D,textureId);
    glActiveTexture(GL_TEXTURE_2D);
   GL_BGRA
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,texWidth,texHeight,GL_RGBA,GL_UNSIGNED_BYTE,texture)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,texture);
    glBindTexture(GL_TEXTURE_2D,0);

}
void setVertex(GLint x,GLint y,GLint width,GLint height,GLint max_width,GLint max_height){
    vertexArray[0] = (GLfloat)(2 * x) / (GLfloat)max_width - 1.0f;
    vertexArray[1] = 1.0f - (GLfloat)(2 * (y + height)) / (GLfloat)max_height;
    vertexArray[2] = (GLfloat)(2 * (x + width)) / (GLfloat)max_width - 1.0f;
    vertexArray[3] = 1.0f - (GLfloat)(2 * (y + height)) / (GLfloat)max_height;
    vertexArray[4] = (GLfloat)(2 * x) / (GLfloat)max_width - 1.0f;
    vertexArray[5] = 1.0f - (GLfloat)(2 * y) / (GLfloat)max_height;
    vertexArray[6] = (GLfloat)(2 * (x + width)) / (GLfloat)max_width - 1.0f;
    vertexArray[7] = 1.0f - (GLfloat)(2 * y) / (GLfloat)max_height;
}
void closeRenderProg(RENDER *render){

    if (render->primary_texId){
        glDeleteTextures(1,&render->primary_texId);
    }
    if (render->cursor_texId){
        glDeleteTextures(1,&render->cursor_texId);
    }
    if (render->fragmentShaderID) {
        glDeleteShader (render->fragmentShaderID);
        glDetachShader (render->program, render->fragmentShaderID);
    }
    if (render->vertexShaderID) {
        glDeleteShader (render->vertexShaderID);
        glDetachShader (render->program, render->vertexShaderID);
    }
    if (render->program){
        glDeleteProgram (render->program);
    }
    g_free(render);
}