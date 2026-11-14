#include <iostream>
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

//define window dimensions
constexpr int WINDOW_WIDTH{700};
constexpr int WINDOW_HEIGHT{800};

//define SDL Window related variables
SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};
SDL_Event currentEvent;
SDL_Color backgroundColor{255, 255, 255, 255};

bool quit{false};

float displayScale{1.0f};

#define MAX_GLOANTE 200
#define NR_INAMICI 3

float W, H;


SDL_FRect nava = {0, 0, 50, 50};
float px, py;          // centrul navei
float vx = 0, vy = 0;  // viteza navei
float dx = 0, dy = 0;  // cat s-a miscat mouse-ul intre frame-uri
float acc = 400;
float vmax = 500;

struct Glont {
    float x, y;
    float vx, vy;
};

Glont gloante[MAX_GLOANTE];
int nrGloante = 0;
Glont gloanteInamic[MAX_GLOANTE];
int nrGloanteInamic = 0;

float raza = 10;
float vitezaGlont = 500;
bool trage = false;
float cooldown = 0;

struct Inamic {
    float x, y;
    float v;
    float timer;
    bool viu;
};

Inamic inamici[NR_INAMICI];
float timerSpawn = 3;

int stare = 0;

bool initWindow() {
    bool success{true};

    //Try to initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    } else {
        //Try to create the window and renderer
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "SDL Hello World Example",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
        } else {
            //Apply global display scaling to renderer
            SDL_SetRenderScale(renderer, displayScale, displayScale);

            SDL_SetRenderVSync(renderer, 1);
            SDL_SetWindowRelativeMouseMode(window, true);

            //Set background color
            SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                                   backgroundColor.a);

            //Apply background color
            SDL_RenderClear(renderer);
        }
    }

    return success;
}

void marime() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    W = w / displayScale;
    H = h / displayScale;
}

void adauga(Glont v[], int &n, float x, float y, float vitx, float vity) {
    if (n >= MAX_GLOANTE)
        return;
    v[n].x = x;
    v[n].y = y;
    v[n].vx = vitx;
    v[n].vy = vity;
    n++;
}

void sterge(Glont v[], int &n, int i) {
    v[i] = v[n - 1];
    n--;
}

void spawn(int i) {
    float latime = W / NR_INAMICI;
    float jum = nava.w / 2;
    inamici[i].x = i * latime + jum + rand() % (int)(latime - 2 * jum);
    inamici[i].y = -jum;
    inamici[i].v = 50 + rand() % 101;
    inamici[i].timer = 1;
    inamici[i].viu = true;
}

void reset() {
    marime();
    px = W / 2;
    py = H - nava.h / 2;
    vx = 0;
    vy = 0;
    dx = 0;
    dy = 0;

    nrGloante = 0;
    nrGloanteInamic = 0;
    trage = false;
    cooldown = 0;

    for (int i = 0; i < NR_INAMICI; i++)
        spawn(i);
    timerSpawn = 3;

    stare = 0;
}

void repulsor() {
    for (int i = 0; i < nrGloanteInamic; i++) {
        float ax = gloanteInamic[i].x - px;
        float ay = gloanteInamic[i].y - py;
        float d = sqrt(ax * ax + ay * ay);
        if (d == 0)
            continue;
        gloanteInamic[i].vx += ax / d * 10;
        gloanteInamic[i].vy += ay / d * 10;
    }
}

void processEvents() {
    while (SDL_PollEvent(&currentEvent)) {
        //User requests quit
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        //Mouse event -> pressed button
        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT)
                trage = true;
            if (currentEvent.button.button == SDL_BUTTON_RIGHT && stare == 0)
                repulsor();
        }

        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT)
                trage = false;
        }

        //Mouse event -> mouse movement
        if (currentEvent.type == SDL_EVENT_MOUSE_MOTION) {
            dx += currentEvent.motion.xrel;
            dy += currentEvent.motion.yrel;
        }

        //Keyboard event
        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            switch (currentEvent.key.key) {
                case SDLK_R:
                    if (stare != 0)
                        reset();
                    break;
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                default:
                    break;
            }
        }
    }
}

void miscaNava(float dt) {
    if (dx != 0 || dy != 0) {
        vx += dx * acc * dt;
        vy += dy * acc * dt;
    } else {
        float v = sqrt(vx * vx + vy * vy);
        if (v > 0) {
            float v2 = v - 2 * acc * dt;
            if (v2 < 0)
                v2 = 0;
            vx = vx * v2 / v;
            vy = vy * v2 / v;
        }
    }
    dx = 0;
    dy = 0;

    float v = sqrt(vx * vx + vy * vy);
    if (v > vmax) {
        vx = vx * vmax / v;
        vy = vy * vmax / v;
    }

    px += vx * dt;
    py += vy * dt;

    float jum = nava.w / 2;
    if (px < jum) {
        px = jum;
        vx = 0;
    }
    if (px > W - jum) {
        px = W - jum;
        vx = 0;
    }
    if (py < jum) {
        py = jum;
        vy = 0;
    }
    if (py > H - jum) {
        py = H - jum;
        vy = 0;
    }
}

bool afara(Glont g) {
    if (g.x < -raza || g.x > W + raza)
        return true;
    if (g.y < -raza || g.y > H + raza)
        return true;
    return false;
}

void miscaGloante(float dt) {
    cooldown -= dt;
    if (trage && cooldown <= 0) {
        adauga(gloante, nrGloante, px, py, 0, -vitezaGlont);
        cooldown = 0.3f;
    }

    for (int i = 0; i < nrGloante; i++) {
        gloante[i].x += gloante[i].vx * dt;
        gloante[i].y += gloante[i].vy * dt;
    }
    for (int i = 0; i < nrGloanteInamic; i++) {
        gloanteInamic[i].x += gloanteInamic[i].vx * dt;
        gloanteInamic[i].y += gloanteInamic[i].vy * dt;
    }

    for (int i = nrGloante - 1; i >= 0; i--)
        if (afara(gloante[i]))
            sterge(gloante, nrGloante, i);
    for (int i = nrGloanteInamic - 1; i >= 0; i--)
        if (afara(gloanteInamic[i]))
            sterge(gloanteInamic, nrGloanteInamic, i);
}

void trageInamic(int i) {
    float ax = px - inamici[i].x;
    float ay = py - inamici[i].y;
    float d = sqrt(ax * ax + ay * ay);
    if (d == 0)
        return;

    float v = vitezaGlont / 5;
    float ux = ax / d * v;
    float uy = ay / d * v;

    float unghi = 15 * 3.14159f / 180;
    float c = cos(unghi);
    float s = sin(unghi);

    float x = inamici[i].x;
    float y = inamici[i].y;
    adauga(gloanteInamic, nrGloanteInamic, x, y, ux, uy);
    adauga(gloanteInamic, nrGloanteInamic, x, y, ux * c - uy * s, ux * s + uy * c);
    adauga(gloanteInamic, nrGloanteInamic, x, y, ux * c + uy * s, -ux * s + uy * c);
}

void miscaInamici(float dt) {
    float jum = nava.w / 2;

    for (int i = 0; i < NR_INAMICI; i++) {
        if (!inamici[i].viu)
            continue;

        inamici[i].y += inamici[i].v * dt;

        if (inamici[i].y > H + jum) {
            inamici[i].y = -jum;
            inamici[i].timer = 1;
        }

        inamici[i].timer -= dt;
        if (inamici[i].timer <= 0) {
            trageInamic(i);
            inamici[i].timer = 3;
        }
    }

    timerSpawn -= dt;
    if (timerSpawn <= 0) {
        for (int i = 0; i < NR_INAMICI; i++)
            if (!inamici[i].viu)
                spawn(i);
        timerSpawn = 3;
    }
}

bool atinge(float cx, float cy, float x, float y) {
    float jum = nava.w / 2;
    float nx = cx;
    float ny = cy;
    if (nx < x - jum) nx = x - jum;
    if (nx > x + jum) nx = x + jum;
    if (ny < y - jum) ny = y - jum;
    if (ny > y + jum) ny = y + jum;
    float ax = cx - nx;
    float ay = cy - ny;
    return ax * ax + ay * ay <= raza * raza;
}

void coliziuni() {
    for (int i = nrGloante - 1; i >= 0; i--) {
        for (int j = 0; j < NR_INAMICI; j++) {
            if (inamici[j].viu && atinge(gloante[i].x, gloante[i].y, inamici[j].x, inamici[j].y)) {
                inamici[j].viu = false;
                sterge(gloante, nrGloante, i);
                break;
            }
        }
    }

    for (int j = 0; j < NR_INAMICI; j++) {
        if (!inamici[j].viu)
            continue;
        if (fabs(px - inamici[j].x) < nava.w && fabs(py - inamici[j].y) < nava.h)
            stare = 2;
    }

    for (int i = 0; i < nrGloanteInamic; i++)
        if (atinge(gloanteInamic[i].x, gloanteInamic[i].y, px, py))
            stare = 2;

    if (stare == 2)
        return;

    bool gata = true;
    for (int j = 0; j < NR_INAMICI; j++)
        if (inamici[j].viu)
            gata = false;
    if (gata)
        stare = 1;
}

void update(float dt) {
    if (stare != 0) {
        dx = 0;
        dy = 0;
        return;
    }

    marime();
    miscaNava(dt);
    miscaGloante(dt);
    miscaInamici(dt);
    coliziuni();

    if (stare != 0) {
        nrGloante = 0;
        nrGloanteInamic = 0;
        for (int i = 0; i < NR_INAMICI; i++)
            inamici[i].viu = false;
    }
}

void cerc(float cx, float cy, float r) {
    for (float y = -r; y <= r; y++) {
        float x = sqrt(r * r - y * y);
        SDL_RenderLine(renderer, cx - x, cy + y, cx + x, cy + y);
    }
}

void drawFrame() {
    //Clear the background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    //Draw scene
    SDL_FRect r = {0, 0, nava.w, nava.h};
    for (int i = 0; i < NR_INAMICI; i++) {
        if (!inamici[i].viu)
            continue;
        r.x = inamici[i].x - r.w / 2;
        r.y = inamici[i].y - r.h / 2;
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 150, 90, 0, 255);
        SDL_RenderRect(renderer, &r);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < nrGloante; i++)
        cerc(gloante[i].x, gloante[i].y, raza);

    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    for (int i = 0; i < nrGloanteInamic; i++)
        cerc(gloanteInamic[i].x, gloanteInamic[i].y, raza);

    nava.x = px - nava.w / 2;
    nava.y = py - nava.h / 2;

    if (stare == 0)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    else if (stare == 1)
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    else
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &nava);

    //Update window
    SDL_RenderPresent(renderer);
}

void cleanup() {
    //Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    //Destroy window
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    //Quit SDL
    SDL_Quit();
}

int main() {
    //Initialize window
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    SDL_zero(currentEvent);

    srand(time(NULL));
    reset();

    Uint64 timpVechi = SDL_GetTicks();

    //Game loop
    while (!quit) {
        Uint64 timp = SDL_GetTicks();
        float dt = (timp - timpVechi) / 1000.0f;
        timpVechi = timp;

        processEvents();
        update(dt);
        drawFrame();
    }

    cleanup();
    return 0;
}
