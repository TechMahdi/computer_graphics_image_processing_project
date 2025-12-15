// ===================== Mahdi islam chowdhury =====================
// REALISTIC 3D Solar System + Colorful Asteroid Belt
#include <GL/glut.h>
#include <cmath>
#include <string>
#include <cstdlib>
#include <vector>
#include <ctime>
// ---------------- Global controls ----------------
bool isPaused = false;
bool showOrbits = true;
bool showLabels = true;
float speedFactor = 1.0f;
// Camera control
float cameraDistance = 35.0f; // Pulled back slightly to see belt
float cameraAngleX = 30.0f;
float cameraAngleY = 0.0f;
int lastMouseX, lastMouseY;
bool isDragging = false;
// ---------------- Data Structures ----------------
struct Planet {
    float distance; float radius; float rotationSpeed; float orbitSpeed;
    float axisTilt; float orbitIncline; float color[3];
    int moonCount; float moonDistance[5]; float moonRadius[5]; float moonOrbitSpeed[5];
    float currentOrbitAngle; float currentRotationAngle; float moonAngles[5];
};
//  Asteroid Structure
struct Asteroid {
    float distance;
    float angle;
    float orbitSpeed;
    float radius;
    float scale[3]; // For irregularly shaped rocks
    float color[3];
    float spinAxis[3]; // Random tumble axis
};
const int NUM_ASTEROIDS = 800;
std::vector<Asteroid> asteroids;
Planet planets[8] = {
    {3.0f, 0.3f, 1.5f, 4.0f, 0.0f, 7.0f, {0.7f,0.7f,0.7f}, 0}, // Mercury
    {4.5f, 0.4f, -0.5f, 3.0f, 177.0f, 3.4f, {0.9f,0.8f,0.6f}, 0}, // Venus
    {6.0f, 0.5f, 4.0f, 2.0f, 23.5f, 0.0f, {0.2f,0.5f,1.0f}, 1, {1.0f}, {0.12f}, {6.0f}}, // Earth
    {7.5f, 0.4f, 3.8f, 1.5f, 25.0f, 1.8f, {0.8f,0.3f,0.2f}, 2, {0.7f, 1.0f}, {0.06f, 0.05f}, {5.0f, 3.5f}}, // Mars
    // --- Asteroid Belt goes here ---
    {12.5f,1.1f, 8.0f, 0.8f, 3.0f, 1.3f, {0.8f,0.7f,0.5f}, 4, {1.6f,1.9f,2.2f,2.5f}, {0.12f,0.1f,0.13f,0.11f}, {4.0f,3.5f,3.0f,2.5f}}, // Jupiter (moved out slightly)
    {15.5f,0.9f, 7.5f, 0.6f, 26.7f, 2.5f, {0.9f,0.85f,0.6f}, 1, {1.5f}, {0.1f}, {3.0f}}, // Saturn
    {18.5f,0.7f, 5.0f, 0.4f, 97.8f, 0.8f, {0.5f,0.9f,0.9f}, 0}, // Uranus
    {21.5f,0.7f, 5.0f, 0.3f, 28.3f, 1.8f, {0.3f,0.3f,0.8f}, 0}  // Neptune
};

std::string planetNames[8] = {"Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune"};

// ---------------- Initialization Functions ----------------
// NEW: Initialize Colorful Asteroids
void initAsteroids() {
    asteroids.resize(NUM_ASTEROIDS);
    for(int i = 0; i < NUM_ASTEROIDS; i++) {
        // Position between Mars (7.5) and Jupiter (12.5)
        // A thick band between 8.5 and 11.0
        asteroids[i].distance = 8.5f + (rand() % 250) / 100.0f;
        asteroids[i].angle = rand() % 360;
        // Slower orbit further out (Kepler-ish approximation)
        asteroids[i].orbitSpeed = (15.0f / asteroids[i].distance) * ((rand()%10+5)/10.0f);

        asteroids[i].radius = 0.04f + (rand() % 8) / 100.0f;

        // Make shape irregular
        asteroids[i].scale[0] = 0.7f + (rand()%6)/10.0f;
        asteroids[i].scale[1] = 0.7f + (rand()%6)/10.0f;
        asteroids[i].scale[2] = 0.7f + (rand()%6)/10.0f;
        asteroids[i].spinAxis[0] = (rand()%10)/10.0f;
        asteroids[i].spinAxis[1] = (rand()%10)/10.0f;
        asteroids[i].spinAxis[2] = (rand()%10)/10.0f;

        // Generate VARIETY OF COLORS
        int type = rand() % 6;
        switch(type) {
            case 0: // Reddish/Rusty
                asteroids[i].color[0] = 0.6f + (rand()%30)/100.0f; asteroids[i].color[1] = 0.3f + (rand()%20)/100.0f; asteroids[i].color[2] = 0.2f; break;
            case 1: // Grey/Metallic
                asteroids[i].color[0] = 0.4f + (rand()%30)/100.0f; asteroids[i].color[1] = asteroids[i].color[0]; asteroids[i].color[2] = asteroids[i].color[0]+0.1f; break;
            case 2: // Golden/Brown
                asteroids[i].color[0] = 0.7f + (rand()%20)/100.0f; asteroids[i].color[1] = 0.6f + (rand()%20)/100.0f; asteroids[i].color[2] = 0.1f; break;
            case 3: // Icy Blueish
                asteroids[i].color[0] = 0.3f; asteroids[i].color[1] = 0.5f + (rand()%30)/100.0f; asteroids[i].color[2] = 0.8f + (rand()%20)/100.0f; break;
            case 4: // Dark carbonaceous
                 asteroids[i].color[0] = 0.2f + (rand()%10)/100.0f; asteroids[i].color[1] = 0.2f + (rand()%10)/100.0f; asteroids[i].color[2] = 0.2f + (rand()%10)/100.0f; break;
            case 5: // Rare Greenish mineral
                 asteroids[i].color[0] = 0.2f; asteroids[i].color[1] = 0.6f + (rand()%30)/100.0f; asteroids[i].color[2] = 0.3f + (rand()%20)/100.0f; break;
        }
    }
}

// ---------------- Drawing Utility ----------------
void drawSphere(float r, int sl, int st, float cr, float cg, float cb) {
    // Set material properties for realistic planet lighting
    GLfloat mat_ambient[] = { cr*0.2f, cg*0.2f, cb*0.2f, 1.0f };
    GLfloat mat_diffuse[] = { cr, cg, cb, 1.0f };
    GLfloat mat_specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat mat_shininess[] = { 20.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glutSolidSphere(r, sl, st);
}

//   Asteroid Belt
void drawAsteroidBelt() {
    // Asteroids are duller than planets
    GLfloat mat_spec[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat mat_shine[] = { 5.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shine);

    for(const auto& ast : asteroids) {
        glPushMatrix();
        // Orbit rotation
        glRotatef(ast.angle, 0, 1, 0);
        // Position
        glTranslatef(ast.distance, 0, 0);
        // Tumble rotation (makes them sparkle)
        glRotatef(ast.angle*3.0f, ast.spinAxis[0], ast.spinAxis[1], ast.spinAxis[2]);
        // Irregular Scale
        glScalef(ast.scale[0], ast.scale[1], ast.scale[2]);

        // Set Color Material
        GLfloat mat_col[] = {ast.color[0], ast.color[1], ast.color[2], 1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_col);

        // Low poly sphere looks like a rock
        glutSolidSphere(ast.radius, 5, 4);
        glPopMatrix();
    }
}

void drawOrbit(float distance, float inclination) {
    if(!showOrbits) return;
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.2f, 0.3f); // Fainter blueish orbits
    glPushMatrix();
    glRotatef(inclination, 0, 0, 1);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 100; i++) {
        float theta = 2.0f * M_PI * float(i) / 100.0f;
        glVertex3f(distance * cosf(theta), 0, distance * sinf(theta));
    }
    glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawSaturnRings(float planetRadius, float tilt) {
    glDisable(GL_LIGHTING);
    glPushMatrix();
        glRotatef(tilt, 0, 0, 1);
        glRotatef(90, 1, 0, 0);
        for(float r = 1.2f; r < 2.2f; r += 0.05f) {
            float alpha = 1.0f - (r - 1.2f);
            if(r > 1.4f && r < 1.5f) continue; // Cassini gap
            glColor4f(0.8f, 0.7f, 0.5f, 0.5f*alpha);
            glutWireTorus(0.02, planetRadius * r, 2, 50);
        }
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawText(float x, float y, float z, std::string text) {
    if(!showLabels) return;
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST); // Always show on top
    glColor3f(1, 1, 1);
    glRasterPos3f(x, y, z);
    for(char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ---------------- Display ----------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float camX = cameraDistance * sin(cameraAngleX * M_PI / 180) * sin(cameraAngleY * M_PI / 180);
    float camY = cameraDistance * cos(cameraAngleX * M_PI / 180);
    float camZ = cameraDistance * sin(cameraAngleX * M_PI / 180) * cos(cameraAngleY * M_PI / 180);
    gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);

    // Stars (Static background)
    glDisable(GL_LIGHTING);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for(int i=0; i<500; i++) {
        glColor3f(0.6f + (rand()%4)/10.0f, 0.6f + (rand()%4)/10.0f, 0.8f);
        glVertex3f(rand()%800-400, rand()%800-400, rand()%800-400);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    // --- SUN ---
    glPushMatrix();
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.9f, 0.4f);
        glutSolidSphere(2.2, 40, 40); // Sun Glow
        glEnable(GL_LIGHTING);
        GLfloat lightPos[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glPopMatrix();

    // --- ASTEROIDS (Drawn before planets) ---
    drawAsteroidBelt();

    // --- PLANETS ---
    for(int i = 0; i < 8; i++) {
        drawOrbit(planets[i].distance, planets[i].orbitIncline);

        glPushMatrix();
            glRotatef(planets[i].orbitIncline, 0, 0, 1);
            float angleRad = planets[i].currentOrbitAngle * M_PI / 180.0f;
            glTranslatef(planets[i].distance * cos(angleRad), 0, planets[i].distance * sin(angleRad));

            drawText(0, planets[i].radius + 0.5f, 0, planetNames[i]);

            glPushMatrix();
                glRotatef(planets[i].axisTilt, 0, 0, 1);
                glRotatef(planets[i].currentRotationAngle, 0, 1, 0);
                drawSphere(planets[i].radius, 25, 25, planets[i].color[0], planets[i].color[1], planets[i].color[2]);
            glPopMatrix();

            if(i == 5) drawSaturnRings(planets[i].radius, planets[i].axisTilt);

            for(int m = 0; m < planets[i].moonCount; m++) {
                glPushMatrix();
                    glRotatef(planets[i].axisTilt, 0,0,1);
                    glRotatef(planets[i].moonAngles[m], 0, 1, 0);
                    glTranslatef(planets[i].moonDistance[m], 0, 0);
                    drawSphere(planets[i].moonRadius[m], 8, 8, 0.6f, 0.6f, 0.6f);
                glPopMatrix();
            }
        glPopMatrix();
    }
    glutSwapBuffers();
}

// ---------------- Update ----------------
void update(int value) {
    if(!isPaused) {
        // Update Planets
        for(int i = 0; i < 8; i++) {
            planets[i].currentOrbitAngle += planets[i].orbitSpeed * speedFactor;
            planets[i].currentRotationAngle += planets[i].rotationSpeed * speedFactor * 5.0f;
            for(int m = 0; m < planets[i].moonCount; m++) {
                planets[i].moonAngles[m] += planets[i].moonOrbitSpeed[m] * speedFactor;
            }
        }
        // Update Asteroids
        for(auto& ast : asteroids) {
            ast.angle += ast.orbitSpeed * speedFactor;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ---------------- Input & Setup ----------------
void keyboard(unsigned char key, int x, int y) {
    if(key == 27) exit(0);
    if(key == 'p' || key == 'P') isPaused = !isPaused;
    if(key == 'o' || key == 'O') showOrbits = !showOrbits;
    if(key == 'l' || key == 'L') showLabels = !showLabels;
    if(key == '+') speedFactor += 0.1f;
    if(key == '-') { speedFactor -= 0.1f; if(speedFactor < 0) speedFactor = 0; }
}

void mouse(int btn, int state, int x, int y) {
    if(btn == GLUT_LEFT_BUTTON) {
        if(state == GLUT_DOWN) { isDragging = true; lastMouseX = x; lastMouseY = y; }
        else isDragging = false;
    }
    if(btn == 3) cameraDistance -= 1.0f;
    if(btn == 4) cameraDistance += 1.0f;
}

void motion(int x, int y) {
    if(isDragging) {
        cameraAngleY += (x - lastMouseX); cameraAngleX += (y - lastMouseY);
        lastMouseX = x; lastMouseY = y;
    }
}

void reshape(int w, int h) {
    if(h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w / h, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    srand(time(0)); // Seed random numbers
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Brighter sun light
    GLfloat lightAmb[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat lightDiff[] = {1.2f, 1.1f, 1.0f, 1.0f};
    GLfloat lightSpec[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    // Realistic light fade over distance
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.8f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.008f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.001f);

    glClearColor(0.02f, 0.02f, 0.05f, 1.0f); // Dark space background

    // Initialize objects
    for(int i=0; i<8; i++) planets[i].currentOrbitAngle = rand()%360;
    initAsteroids();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Realistic Solar System + Colorful Asteroids");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
