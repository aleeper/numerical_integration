// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// A pretty standard vertex program that transforms normals, tangents (to
// support bump mapping), and viewing direction.  Some texture coordinates
// are also passed along.
//
// Author:  Sonny Chan
// Date:    June 2009
// --------------------------------------------------------------------------

attribute vec3 tangent;     // consistent tangent as vertex attribute

varying vec3 tnorm;         // transformed surface normal
varying vec3 ttang;         // transformed surface tangent
varying vec3 eyedir;        // viewing direction

// --------------------------------------------------------------------------

void main(void)
{
    // transform normal and tangent, compute viewing direction
    tnorm = normalize(gl_NormalMatrix * gl_Normal);
    ttang = normalize(gl_NormalMatrix * tangent);
    vec4 position = gl_ModelViewMatrix * gl_Vertex;
    eyedir = normalize(position.xyz);

    // pass along texture coordinates
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;

    // do the fixed-function transform on the vertex
    gl_Position = ftransform();

    // just for testing -- when fragment shader fails, object is blue-ish
    gl_FrontColor = vec4(.2, .4, .6, .8);
}

// --------------------------------------------------------------------------
