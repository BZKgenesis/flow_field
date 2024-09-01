#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdio>
#include <vector>

const int H = 1024;
const int W = 1024;

int vectorLengt = 15;

bool hideVector = true;

float mouvementSpeed = 10;


//2D Perlin Noise
int nOutputWidth = 64;
int nOutputHeight = 64;
float* fNoiseSeed2D = nullptr;
float* fPerlinNoise2D = nullptr;
std::vector<sf::Vector2f> vector_field;

int maxOctave = 8;

//1D Perlin Noise
int arraySize = 256;
float *fNoiseSeed1D = nullptr;
float *fPerlinNoise1D = nullptr;


int OctaveCount = 2;
float fScalingBias = 3.0f;
int nMode = 2;

class Particle {
public:
    bool visible = true;
    sf::Vector2f pos;
    sf::Vector2f vel;
    float size;
    sf::Vector2f acc = sf::Vector2f(0, 0);
    sf::Color color = sf::Color(255,0,0,100);
    Particle(sf::Vector2f position, sf::Vector2f velocity, float size) : pos(position), vel(velocity), size(size) {};


    void draw(sf::RenderWindow& win) const {
        sf::CircleShape circle = sf::CircleShape(size);
        circle.setPosition(pos);
        circle.setFillColor(color);
        circle.setOutlineColor(color);
        sf::VertexArray line = sf::VertexArray(sf::Lines);
        line.append(pos + sf::Vector2f(size, size));
        line.append(pos + sf::Vector2f(size, size) +(acc*5.0f));

        win.draw(circle);
        //win.draw(line);
    }

    void update(float dt) {
		//std::cout << (int)((pos.y / H) * nOutputHeight) * nOutputWidth + (int)((pos.x / W) * nOutputWidth) << std::endl;
		//std::cout << vector_field.size() << std::endl;
        if ((int)((pos.y / H) * nOutputHeight) * nOutputWidth + (int)((pos.x / W) * nOutputWidth) > 4095)
            return;
        acc = vector_field[(int)((pos.y / H) * nOutputHeight) * nOutputWidth + (int)((pos.x / W) * nOutputWidth)] * mouvementSpeed;
		//std::cout << acc.x << "   " << acc.y << std::endl;
        vel += acc * dt;
        float lenght = sqrt(vel.x * vel.x + vel.y * vel.y);
        if (lenght > 20) {
            float lenght = sqrt(vel.x * vel.x + vel.y * vel.y);
            vel.x = (vel.x / lenght) * 20;
            vel.y = (vel.y / lenght) * 20;
        }
        pos += vel * dt;
        if (pos.x > W) {
            pos.x = 0;
        }if (pos.x < -(size*2)) {
            pos.x = W;
        }
        if (pos.y > H) {
            pos.y = 0;
        }if (pos.y < -(size * 2)) {
            pos.y = H;
        }
    }
};

static void PerlinNoise1D(int nCount, float *fSeed, int nOctaves,float fBiaf ,float *fOutput) {
    for (int x = 0; x < nCount; x++) {
        float fNoise = 0.0f;
        float fScale = 1.0f;
        float fScaleAcc = 0.0f;

        for (int o = 0; o < nOctaves; o++) {
            int nPitch = nCount >> o;
            int nSample1 = (x / nPitch) * nPitch;
            int nSample2 = (nSample1 + nPitch) % nCount;

            float fBlend = (float)(x - nSample1) / (float)nPitch;
            float fSample = (1.0f - fBlend) * fSeed[nSample1] + fBlend * fSeed[nSample2];
            fNoise += fSample * fScale;
            fScaleAcc += fScale;
            fScale = fScale / fBiaf;

        }
        fOutput[x] = fNoise / fScaleAcc;
    }
}

static void PerlinNoise2D(int nWidth,int nHeight, float* fSeed, int nOctaves, float fBiaf, std::vector<sf::Vector2f>& vector_field) {
    for (int x = 0; x < nWidth; x++) {
        for (int y = 0; y < nHeight; y++) {
            float fNoise = 0.0f;
            float fScale = 1.0f;
            float fScaleAcc = 0.0f;

            for (int o = 0; o < nOctaves; o++) {
                int nPitch = nWidth >> o;
                int nSampleX1 = (x / nPitch) * nPitch;
                int nSampleY1 = (y / nPitch) * nPitch;

                int nSampleX2 = (nSampleX1 + nPitch) % nWidth;
                int nSampleY2 = (nSampleY1 + nPitch) % nWidth;

                float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
                float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

                float fSampleT = (1.0f - fBlendX) * fSeed[nSampleY1 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY1 * nWidth + nSampleX2];
                float fSampleB = (1.0f - fBlendX) * fSeed[nSampleY2 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY2 * nWidth + nSampleX2];


                fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
                fScaleAcc += fScale;
                fScale = fScale / fBiaf;

            }
            float val = fNoise / fScaleAcc;
            //std::cout << x*nWidth + y << "   " << vector_field.size() << std::endl;
            vector_field.push_back( sf::Vector2f(cos(val * 3.1415f * 2), sin(val * 3.1415f * 2)));
        }
    }
}

int main()
{
    while (pow(2, maxOctave) <= nOutputHeight) {
        maxOctave++;
    }
    sf::Clock deltaClock;

    //vector_field.reserve(nOutputWidth * nOutputHeight + 1);

    fNoiseSeed1D = new float[arraySize];
    fPerlinNoise1D = new float[arraySize];
    for (int i = 0; i < arraySize; i++) fNoiseSeed1D[i] = (float)rand() / (float)RAND_MAX;

    fNoiseSeed2D = new float[nOutputWidth * nOutputHeight];
    fPerlinNoise2D = new float[nOutputWidth * nOutputHeight];
    for (int i = 0; i < nOutputWidth * nOutputHeight; i++) fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;

    int radius = 100.0f;
    sf::CircleShape circle = sf::CircleShape(radius);
    circle.setFillColor(sf::Color::Green);
    circle.setPosition(sf::Vector2f((W / 2)- radius, (H / 2) - radius));
    sf::RenderWindow window(sf::VideoMode(W, H), "Flow Field");
    //window.setFramerateLimit(600);

    int nbParticle = 1000;

    std::vector<Particle> particules;
    for (int i = 0; i < nbParticle; i++) {
        sf::Vector2f p = sf::Vector2f(rand() % W + 1, rand() % H + 1);
        sf::Vector2f v = sf::Vector2f(0,0);
        particules.push_back(Particle(p, v, 2));
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                OctaveCount++;
                if (OctaveCount >= maxOctave)
                    OctaveCount = 1;
                std::cout << OctaveCount << std::endl;
                
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                fScalingBias -= 0.2f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                fScalingBias += 0.2f;
            }
            if (fScalingBias < 0.2f) {
                fScalingBias = 0.2;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                nMode = 1;
            }if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                nMode = 2;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) {
                hideVector = !hideVector;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Add)) {
                mouvementSpeed -= 0.2f;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract)) {
                mouvementSpeed  += 0.2f;
            }
            if (mouvementSpeed < 0.2f) {
                mouvementSpeed = 0.2f;
            }

        }
        sf::Time dt = deltaClock.restart();

        //window.clear();
        float deltaT = dt.asSeconds();
        if (nMode == 1) {
            PerlinNoise1D(arraySize, fNoiseSeed1D, OctaveCount, fScalingBias, fPerlinNoise1D);
            sf::VertexArray vArray(sf::LinesStrip);
            for (int x = 0; x < arraySize; x++) {
                sf::Vertex vertex = sf::Vertex(sf::Vector2f(x * W/(arraySize), (fPerlinNoise1D[x] * (float)H / 2.0f) + (float)H / 2.0f));
                vArray.append(vertex);
            }
            window.draw(vArray);
        }
        else if (nMode == 2) {
            PerlinNoise2D(nOutputWidth, nOutputHeight, fNoiseSeed2D, OctaveCount, fScalingBias, vector_field);
            if (!hideVector) {
                sf::VertexArray vArray = sf::VertexArray(sf::Lines);
                for (int x = 0; x < nOutputWidth; x++) {
                    for (int y = 0; y < nOutputHeight; y++) {
                        //sf::RectangleShape rect(sf::Vector2f((W/nOutputWidth), (H / nOutputHeight)));
                        //rect.setPosition(sf::Vector2f((W / nOutputWidth) * x, (H / nOutputHeight) * y));
                        //int colorVal = (int)(fPerlinNoise2D[y * nOutputWidth + x] * 255);
                        //rect.setFillColor(sf::Color(colorVal, colorVal, colorVal));
                        //window.draw(rect);
                        vArray.append(sf::Vector2f(x * (W / nOutputWidth), y * (H / nOutputHeight)));
                        vArray.append(sf::Vector2f(x * (W / nOutputWidth) + vectorLengt * vector_field[y * nOutputWidth + x].x, y * (H / nOutputHeight) + vectorLengt * vector_field[y * nOutputWidth + x].y));
                    }
                }
                window.draw(vArray);
            }
        }
        for (int i = 0; i < nbParticle; i++) {
            if (particules[i].visible) {
                particules[i].update(deltaT*10);
                particules[i].draw(window);
            }
        }
       
        window.display();
    }


}
