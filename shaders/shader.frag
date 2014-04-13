// --------------------------------------------------------------------------
// Sonny's Qt+OpenGL Boilerplate Application
//
// GLSL fragment program to compute an environment- and bump-mapped Julia set
//
// Author:  Sonny Chan
// Date:    June 2009
// --------------------------------------------------------------------------

varying vec3 tnorm;                 // transformed surface normal
varying vec3 ttang;                 // transformed surface tangent
varying vec3 eyedir;                // viewing direction

uniform samplerCube environment;    // cube environment map
uniform vec2 c;                     // Julia set constant

const vec2 ds = vec2(0.002, 0.000); // delta-s in complex space for derivative
const vec2 dt = vec2(0.000, 0.002); // delta-t in complex space for derivative
const int iterations = 128;         // maximum number of iterations for Julia

// --------------------------------------------------------------------------
// Computes the escape velocity of a coordinate on the Julia set.  Result is
// in the range [0,1), slower escape velocities are higher, and points within
// the set itself will return 0.0.

float julia(vec2 z)
{
    int i = 0;
    for ( ; i < iterations; ++i)
    {
        // test for escape
        if (dot(z,z) > 10000.0) break;

        // compute Julia set function f(Z) = Z^Z + C
        float real = z.x*z.x - z.y*z.y + c.x;
        float imag = 2.0 * z.x*z.y + c.y;
        z = vec2(real, imag);
    }

    return fract(float(i)/float(iterations));
}

// --------------------------------------------------------------------------

void main(void)
{
    // initial location on complex plane based on (multi)texture coordinates
    vec2 z = gl_TexCoord[1].xy * vec2(4.0, 2.0) - vec2(2.0, 1.0);
    
    // compute the escape speed of this complex coordinate
    float speed = julia(z);

    // use numerical derivative to compute normal perturbation
    float nds = 4.0 * (julia(z+ds) - julia(z-ds));
    float ndt = 4.0 * (julia(z+dt) - julia(z-dt));
    vec3 binorm = cross(tnorm, ttang);
    vec3 normal = normalize(tnorm - nds*ttang - ndt*binorm);

    // reflect the incident direction about the normal to get environment ray
    vec3 r = reflect(eyedir, normal);

    // the inverse camera transform is stored in texture matrix 0
    r = (gl_TextureMatrix[0] * vec4(r, 1.0)).xyz;

    // colour the fractal yellow based on escape speed
    vec4 material = vec4(4.0*speed, 4.0*speed, 2.0*speed, 1.0);

    // then combine with environment-mapped reflection
    vec4 reflection = textureCube(environment, r);
    gl_FragColor = mix(material, reflection, 0.25);
}

// --------------------------------------------------------------------------
