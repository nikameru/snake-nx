#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <time.h>

#include <switch.h>

/* Field dimensions */
#define FIELD_HEIGHT 8
#define FIELD_WIDTH 10
/* Field update interval (in frames) */
#define SNAKE_SPEED 30

/* Game field (2D vector) */
typedef std::vector<std::vector<std::string>> Field;
/* Snake blocks coordinates (2D vector) */
typedef std::vector<std::vector<int>> Coordinates;

/* Game variables declaration */
int framesPassed;
bool gameContinues;

void renderField(Field &field)
{
    int height = field.size();

    for (int i = 0; i < height; i++)
    {
        int width = field[i].size();

        for (int j = 0; j < width; j++)
        {
            std::cout << field[i][j] << " ";
        }

        std::cout << std::endl;
    }
}

void placeFruit(Field &field)
{
    int x, y;

    /* Generate random coordinates */
    do
    {
        x = rand() % FIELD_HEIGHT;
        y = rand() % FIELD_WIDTH;
    } while (field[x][y] == "*");

    /* Placing a fruit on the field */
    field[x][y] = "@";
}

void gameOver()
{
    gameContinues = false;
}

/* Snake directions: 0 (^), 1 (>), 2 (v), 3 (<) */
class Snake
{
private:
    /* Coordinates for each block */
    Coordinates coordinates = {};
    /* Snake is heading downwards by default */
    int direction = 2, length = 3;

    /* Check next tile for collisions, fruits etc. */
    void processDestination(Field &field, int x, int y)
    {
        /* Check for walls (+ 1 because they are indices) */
        if (x < 0 || x + 1 > FIELD_HEIGHT)
            gameOver();
        else if (y < 0 || y + 1 > FIELD_WIDTH)
            gameOver();
        /* Check for body collision */
        else if (field[x][y] == "*")
            gameOver();
        else
        {
            /* Check for fruit collision */
            if (field[x][y] == "@")
            {
                eatFruit();
                placeFruit(field);
            }
            else
            {
                /* Delete the last block (no collisions/fruits) */
                field[coordinates.back()[0]][coordinates.back()[1]] = ".";
                coordinates.erase(coordinates.end());
            }

            /* Move the first block */
            field[x][y] = "*";
            coordinates.insert(coordinates.begin(), {x, y});
        }
    }

public:
    Snake()
    {
        /* Empty constructor */
    }

    /* Change snake position on the field, check for game over etc. */
    void move(Field &field)
    {
        switch (direction)
        {
        case 0:
            processDestination(field, coordinates[0][0] - 1, coordinates[0][1]);
            break;
        case 1:
            processDestination(field, coordinates[0][0], coordinates[0][1] + 1);
            break;
        case 2:
            processDestination(field, coordinates[0][0] + 1, coordinates[0][1]);
            break;
        case 3:
            processDestination(field, coordinates[0][0], coordinates[0][1] - 1);
            break;
        }

        /* Re-render field */
        consoleClear();
        renderField(field);
    }

    /* Direction changes handling */
    void adjustDirection(u64 kDown)
    {
        switch (direction)
        {
        case 0:
            if (kDown & HidNpadButton_AnyLeft)
                turnLeft();
            else if (kDown & HidNpadButton_AnyRight)
                turnRight();
            break;
        case 1:
            if (kDown & HidNpadButton_AnyUp)
                turnLeft();
            else if (kDown & HidNpadButton_AnyDown)
                turnRight();
            break;
        case 2:
            if (kDown & HidNpadButton_AnyRight)
                turnLeft();
            else if (kDown & HidNpadButton_AnyLeft)
                turnRight();
            break;
        case 3:
            if (kDown & HidNpadButton_AnyDown)
                turnLeft();
            else if (kDown & HidNpadButton_AnyUp)
                turnRight();
            break;
        }
    }

    /* Direction encapsulators */
    void turnLeft()
    {
        if (direction - 1 != -1)
            direction--;
        else
            direction = 3;
    }

    void turnRight()
    {
        if (direction + 1 != 4)
            direction++;
        else
            direction = 0;
    }

    /* Length encapsulators */
    int getLength()
    {
        return length;
    }

    void eatFruit()
    {
        length++;
    }

    /* Coordinates encapsulators */
    Coordinates getCoordinates()
    {
        return coordinates;
    }

    void setCoordinates(Coordinates newCoordinates)
    {
        coordinates = newCoordinates;
    }

    /* Reset to default values (on game restart) */
    void resetProperties()
    {
        coordinates = {};
        direction = 2;
        length = 3;
    }
};

Field createField(int height, int width, Snake &snake)
{
    /* Creating empty field with dots */
    Field field(height, std::vector<std::string>(width));

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            field[i][j] = ".";
        }
    }

    /* Placing snake in its start position */
    for (int i = 0; i < snake.getLength(); i++)
    {
        field[i + 1][FIELD_WIDTH / 2] = "*";
        /* Setting coordinates */
        Coordinates coordinates = snake.getCoordinates();

        coordinates.insert(coordinates.begin(), {i + 1, FIELD_WIDTH / 2});

        snake.setCoordinates(coordinates);
    }

    /* Placing one fruit on the field in advance as well */
    placeFruit(field);

    return field;
}

void gameStart(Snake &snake, Field &field)
{
    /* Just resetting everything */
    snake.resetProperties();

    field = createField(FIELD_HEIGHT, FIELD_WIDTH, snake);
    renderField(field);

    framesPassed = 0;
    gameContinues = true;
}

/* Game entrypoint */
int main()
{
    /* Console init */
    consoleInit(NULL);

    /* Player input init */
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    /* Seeding random numbers generator */
    srand(time(NULL));

    /* Game init */
    gameContinues = false;

    Snake snake;
    Field field;

    field = createField(FIELD_HEIGHT, FIELD_WIDTH, snake);

    /* Telling about game controls */
    std::cout << "(A) Start / restart." << std::endl << "(+) Exit."; 

    while (appletMainLoop())
    {
        /* Input processing */
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break;
        else if (kDown & HidNpadButton_A)
            gameStart(snake, field);

        /* Field updates */
        if (gameContinues)
        {
            snake.adjustDirection(kDown);

            /* Checking if the desired amount of frames had passed */
            if (framesPassed == 0)
            {
                snake.move(field);
            }
            else if (framesPassed == SNAKE_SPEED)
            {
                snake.move(field);

                framesPassed = 0;
            }

            /* Increment number of frames */
            framesPassed++;
        }

        /* Update console */
        consoleUpdate(NULL);
    }

    /* Exit */
    consoleExit(NULL);

    return 0;
}
