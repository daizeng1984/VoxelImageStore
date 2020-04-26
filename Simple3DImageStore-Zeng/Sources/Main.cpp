#include <GL/glew.h>
#include <stdio.h>

// All headers are automatically included from "iglu.h"
#include "iglu.h"
using namespace iglu;

// These are handles to the two windows, should you need to modify their
//   behavior after window creation.
IGLUWindow::Ptr         myWin = 0;
IGLUWidgetWindow::Ptr   uiWin = 0;

IGLUBool                displayMode (false, "Change the display mode for voxel: Transparent or ambient occlussion?");
IGLUInt                 marchStepNum            ( 512,   IGLURange<int>(1,512),  1,      "Ray marching step size" );

//The scene geometry consists of two objects: a wavefront obj file and a vertex array for square
IGLUOBJReader::Ptr      cube     = 0;
IGLUOBJReader::Ptr      waveObj     = 0;
IGLUVertexArray::Ptr    vertexGeom = 0;

IGLUShaderProgram::Ptr  render3DTextureShader = 0;
IGLUShaderProgram::Ptr  renderVoxelShader = 0;
IGLUShaderProgram::Ptr  voxelizerShader = 0;

//Trackball for interaction
IGLUTrackball::Ptr		trackBall = 0;

//The framebuffer holding our shadow map
IGLUFramebuffer::Ptr    FBO;

// Location of the light and the eye
vec3            eyePos   = vec3(0.0,0.0,3);

// Matrices for setting up the view from the eye
IGLUMatrix4x4   eyeView = IGLUMatrix4x4::LookAt( eyePos, vec3(0.0,0.0,0.0), vec3::YAxis() );
IGLUMatrix4x4   eyeProj = IGLUMatrix4x4::Perspective( 60, 1, 0.01, 10.0 );

// Matrices for positioning our geometry relative to the world origin

// Texture Index in OpenGL
GLuint g_texName;
GLuint texWidth = 64;
// Temporily now 3d texture
void voxelize(IGLUOBJReader* obj, GLuint tex3DName)
{
    // Draw both CCW and CW triangle
    glDisable(GL_CULL_FACE);
    // Disable all the framebuffer states
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    glDepthMask(GL_FALSE);

    voxelizerShader->Enable();

    // Setup the 3D texture, it's ugly since IGLU does not support it now.
    IGLUShaderVariable& tex3DSampler = voxelizerShader["texImg3D"];
    GLuint tex3DSamplerIdx = tex3DSampler.GetVariableIndex();
    glUniform1i( tex3DSamplerIdx,  tex3DName);
    glBindImageTexture( tex3DName, tex3DName, 0, GL_TRUE, 0, GL_READ_WRITE , GL_RGBA32F);
    voxelizerShader["texWidth"] = float(texWidth);

    obj->Draw();

    voxelizerShader->Disable();

    // Enable all the states that we changed
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}

void displayVoxelUsingRayMarching(GLuint tex3DName)
{

    // view change due to trackball
    IGLUMatrix4x4 viewModel = eyeView * trackBall->GetMatrix();
    render3DTextureShader["view"] = viewModel;
    render3DTextureShader["invViewModel"] = viewModel.Invert();
    render3DTextureShader["model"] = IGLUMatrix4x4::Identity();
    render3DTextureShader["proj"]      = eyeProj;

    // Draw the cube
    render3DTextureShader->Enable();

    // Setup the 3D texture, it's ugly since IGLU does not support it now.
    IGLUShaderVariable& tex3DSampler = render3DTextureShader["texImg3D"];
    GLuint tex3DSamplerIdx = tex3DSampler.GetVariableIndex();
    glUniform1i( tex3DSamplerIdx,  tex3DName);
    glBindImageTexture( tex3DName, tex3DName, 0, GL_TRUE, 0, GL_READ_WRITE , GL_RGBA32F);
    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Setup other parameters
    render3DTextureShader["texWidth"] = float(texWidth);
    render3DTextureShader["marchStepNum"] = float(marchStepNum);

    cube->Draw();

    render3DTextureShader->Disable();

}

void displayVoxelUsingCubebox(GLuint tex3DName)
{
    // Draw multiple cube using 3d texture

    // view change due to trackball
    IGLUMatrix4x4 viewModel = eyeView * trackBall->GetMatrix();
    renderVoxelShader["view"] = viewModel;
    renderVoxelShader["model"] = IGLUMatrix4x4::Identity();
    renderVoxelShader["proj"]      = eyeProj;

    // Draw the cube
    renderVoxelShader->Enable();

    // Setup the 3D texture, it's ugly since IGLU does not support it now.
    glEnable( GL_TEXTURE_3D );
    glActiveTexture( GL_TEXTURE0 +  tex3DName);
    IGLUShaderVariable& tex3DSampler = renderVoxelShader["tex3D"];
    GLuint tex3DSamplerIdx = tex3DSampler.GetVariableIndex();
    glUniform1i( tex3DSamplerIdx,  tex3DName);
    //glBindImageTexture( g_texName, g_texName, 0, GL_TRUE, 0, GL_READ_WRITE , GL_RGBA32F);
    glBindTexture(GL_TEXTURE_3D, tex3DName);

    // Setup other parameters
    renderVoxelShader["voxWidth"] = float(texWidth);

    IGLUVertexArray::Ptr vplObjArray = cube->GetVertexArray();

    vplObjArray->DrawArraysInstanced( GL_TRIANGLES, 0, 3* cube->GetTriangleCount(), texWidth*texWidth*texWidth);

    glBindTexture(GL_TEXTURE_3D, 0);
    glDisable( GL_TEXTURE_3D );
    renderVoxelShader->Disable();
}

void display ( void )	
{
    // Convert mesh to 3D texture
    voxelize(waveObj, g_texName);

    // Display the voxel
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    if(displayMode == true)
        displayVoxelUsingRayMarching(g_texName);
    else
        displayVoxelUsingCubebox(g_texName);

}


// Code that initializes our OpenGL state.  This is guaranteed (by IGLUWindow) to be 
//    called after the OpenGL context & all extensions have been initialized
void OpenGLInitialization( void ){

    //Load an obj file
    waveObj = new IGLUOBJReader("../../CommonSampleFiles-Zeng/Models/arrow.obj", IGLU_OBJ_CENTER | IGLU_OBJ_UNITIZE);
    cube = new IGLUOBJReader( "../../CommonSampleFiles-Zeng/Models/cube.obj", IGLU_OBJ_CENTER | IGLU_OBJ_UNITIZE);

    // Create a virtual trackball
    trackBall = new IGLUTrackball( myWin->w(), myWin->h() );


    // Load a shader that simply render 3d texture to a cube using ray marching
    render3DTextureShader = new IGLUShaderProgram("../Shaders/render3DTexture.vert.glsl", "../Shaders/render3DTexture.frag.glsl");
    render3DTextureShader->SetProgramEnables(IGLU_GLSL_DEPTH_TEST);

    // Load a shader that render cube using 3d texture
    renderVoxelShader = new IGLUShaderProgram("../Shaders/renderVoxelShader.vert.glsl", "../Shaders/renderVoxelShader.frag.glsl");
    renderVoxelShader->SetProgramEnables(IGLU_GLSL_DEPTH_TEST);

    // Load shader that render to 3D textures using image store
    voxelizerShader = new IGLUShaderProgram(  "../Shaders/voxelizer.vert.glsl"
                                            , "../Shaders/voxelizer.geom.glsl"
                                            , "../Shaders/voxelizer.frag.glsl");


    // Create and setup 3D texture
    glEnable(GL_TEXTURE_3D);
    // Create texture index and configuration
    glGenTextures(1, &g_texName);
    glBindTexture(GL_TEXTURE_3D, g_texName);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    GLuint texHeight = texWidth;
    GLuint texDepth = texWidth;

    //Build the raw data for use
    GLubyte* texels = new GLubyte[texWidth*texHeight*texDepth*3];
    memset(texels, 0, texWidth*texHeight*texDepth*3);

    // Draw a sphere 3d texture
    //GLubyte* ptrTexel = texels;
    //for (uint i = 0; i < texWidth; ++ i)
    //{
    //    for (uint j = 0; j < texHeight; ++j)
    //    {
    //        for (uint k = 0; k < texDepth; ++k)
    //        {
    //            float x = i/float(texWidth);
    //            float y = j/float(texHeight);
    //            float z = k/float(texDepth);
    //            vec3 pos(x,y,z);
    //            pos = pos - vec3(0.5,0.5,0.5);
    //            if ( pos.Length() < 0.5 )
    //            {
    //                ptrTexel[ (i + j*texWidth + k*texWidth*texHeight)*3 + 0] = GLubyte(x*255);
    //                ptrTexel[ (i + j*texWidth + k*texWidth*texHeight)*3 + 1] = GLubyte(y*255);
    //                ptrTexel[ (i + j*texWidth + k*texWidth*texHeight)*3 + 2] = GLubyte(z*255);
    //            }
    //        }
    //    }
    //}

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, texWidth, texHeight, texDepth, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, texels);

    glBindTexture(GL_TEXTURE_3D, 0);
    glDisable( GL_TEXTURE_3D );
    // Create shader to convert mesh to voxel (3d texture)
    if(!glewIsSupported("GL_NV_gpu_shader5 GL_ARB_shader_image_load_store"))
        printf("So bad!");


}

// Track any updates to the trackball matrix when the mouse moves 
void Motion(int x, int y)
{
    trackBall->UpdateOnMotion(x, y);
}

// When the user clicks/releases in the main window, start/stop tracking with our trackball
void Button(int button, int state, int x, int y  )
{
    if(IGLU_EVENT_DOWN == state)
        trackBall->SetOnClick( x, y );
    else
        trackBall->Release();
}

int main(int argc, char** argv)
{
    // Create our main window
    myWin = new IGLUWindow( 512, 512, "Simple 3D Image Store" );
    myWin->SetWindowProperties( IGLU_WINDOW_NO_RESIZE |	
        IGLU_WINDOW_DOUBLE |
        IGLU_WINDOW_REDRAW_ON_IDLE |
        IGLU_WINDOW_W_FRAMERATE ); 
    myWin->SetDisplayCallback( display );  
    myWin->SetIdleCallback( IGLUWindow::NullIdle );
    myWin->SetPreprocessOnGLInit( OpenGLInitialization );
    myWin->SetActiveMotionCallback(Motion);
    myWin->SetMouseButtonCallback(Button);
    myWin->CreateWindow( argc, argv );     

    // Create our widget window & add our widgets to it
    uiWin = new IGLUWidgetWindow( 200, 200, "UI Widget Window" );
    uiWin->AddWidget( &marchStepNum );
    uiWin->AddWidget( &displayMode );
    uiWin->AddWidgetSpacer();
    myWin->SetWidgetWindow( uiWin );

    // Start running our IGLU OpenGL program!
    IGLUWindow::Run();
    return 0;
}
