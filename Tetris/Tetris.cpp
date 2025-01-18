#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <utility>
#include <random>

#define SQUARES_QUANTITY_HORIZONTALLY 10
#define SQUARES_QUANTITY_VERTICALLY 20

#define SQUARE_SIDE_LENGTH 40

#define LEFT_OFFSET_FOR_PLAYING_AREA 400
#define TOP_OFFSET_FOR_PLAYING_AREA 40

#define LEFT_OFFSET_FOR_SQUARE_MATRIX 40
#define TOP_OFFSET_FOR_SQUARE_MATRIX 40

#define LEFT_OFFSET_FOR_NEXT_FIGURE 80
#define TOP_OFFSET_FOR_NEXT_FIGURE 80

#define SQUARE_OUTLINE_DIVIDER 20


int windowWidth, windowHeight;

class Square : public sf::RectangleShape
{
public:
    float sideLength;

    Square(sf::Vector2f offsetFromPlayingArea, sf::Vector2f positionIndexes, sf::Color fillColor) : sf::RectangleShape(sf::Vector2f(0, 0)) {
        sideLength = SQUARE_SIDE_LENGTH;
        setSize(sf::Vector2f(sideLength - sideLength / SQUARE_OUTLINE_DIVIDER, sideLength - sideLength / SQUARE_OUTLINE_DIVIDER));
        setPosition(offsetFromPlayingArea.x + positionIndexes.x * sideLength, offsetFromPlayingArea.y + positionIndexes.y * sideLength);
        setFillColor(fillColor);
        setOutlineThickness(sideLength / SQUARE_OUTLINE_DIVIDER);
        setOutlineColor(sf::Color(164, 164, 164));
    }

    ~Square() {}
};

class BaseFigureType
{
public:
    virtual std::vector<Square>& GetSquares() = 0;
    virtual sf::Vector2f GetCurrentPosition() = 0;
    virtual void SetCurrentPosition(sf::Vector2f newCurrentPosition) = 0;

    virtual bool MoveDown(std::vector<Square>& stoppedSquares) = 0;
    virtual bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) = 0;
    virtual void Spin(std::vector<Square>& stoppedSquares) = 0;
    virtual bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) = 0;

    bool MoveDownCommon(std::vector<Square>& squares, sf::Vector2f& currentSquarePosition, std::vector<Square>& stoppedSquares) {
        currentSquarePosition += sf::Vector2f(0, 1);
        bool isCollisionHappend = false;
        if (currentSquarePosition.y - squares[0].getPosition().y >= SQUARE_SIDE_LENGTH) {
            isCollisionHappend = CheckCollision(stoppedSquares, sf::Vector2f(0, SQUARE_SIDE_LENGTH));
            if (!isCollisionHappend) {
                for (int i = 0; i < squares.size(); i++) {
                    squares[i].setPosition(squares[i].getPosition() + sf::Vector2f(0, SQUARE_SIDE_LENGTH));
                }
            }
            currentSquarePosition = squares[0].getPosition();
        }
        return isCollisionHappend;
    }

    bool CheckCollisionCommon(std::vector<Square>& squares, std::vector<Square>& stoppedSquares, sf::Vector2f offset) {
        for (int i = 0; i < squares.size(); i++) {
            for (int j = 0; j < stoppedSquares.size(); j++) {
                if (squares[i].getPosition() + offset == stoppedSquares[j].getPosition()) {
                    return true;
                }
            }
            if (squares[i].getPosition().y + offset.y == TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX + 20 * SQUARE_SIDE_LENGTH) {
                return true;
            }
            if (squares[i].getPosition().x + offset.x == LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX - SQUARE_SIDE_LENGTH ||
                squares[i].getPosition().x + offset.x == LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 10 * SQUARE_SIDE_LENGTH) {
                return true;
            }
        }
        return false;
    }

    void SpinCommon(std::vector<Square>& squares, std::vector<Square>& stoppedSquares, std::vector<std::vector<sf::Vector2f>>& squaresOffsetPositionsDuringRotations,
        int& currentSquaresOffsetPositionsDuringRotations, const std::vector<sf::Vector2f>& offsetDuringRotation,
        int maxQuantityEnteredSquareCollisions) {
        std::vector<Square> spinedSquares = squares;
        if (currentSquaresOffsetPositionsDuringRotations == 3) {
            currentSquaresOffsetPositionsDuringRotations = -1;
        }
        for (int i = 0; i < squaresOffsetPositionsDuringRotations[currentSquaresOffsetPositionsDuringRotations + 1].size(); i++) {
            spinedSquares[i].setPosition(spinedSquares[i].getPosition() + squaresOffsetPositionsDuringRotations[currentSquaresOffsetPositionsDuringRotations + 1][i]);
        }
        for (int i = 0; i < offsetDuringRotation.size(); i++) {
            for (int j = 0; j < maxQuantityEnteredSquareCollisions; j++) {
                if (CheckCollisionForSpin(spinedSquares, stoppedSquares)) {
                    for (int k = 0; k < spinedSquares.size(); k++) {
                        spinedSquares[k].setPosition(spinedSquares[k].getPosition() + offsetDuringRotation[i]);
                    }
                    if (!CheckCollisionForSpin(spinedSquares, stoppedSquares)) {
                        squares = spinedSquares;
                        currentSquaresOffsetPositionsDuringRotations += 1;
                        return;
                    }
                }
                else {
                    squares = spinedSquares;
                    currentSquaresOffsetPositionsDuringRotations += 1;
                    return;
                }
            }
            for (int k = 0; k < spinedSquares.size(); k++) {
                spinedSquares[k].setPosition(spinedSquares[k].getPosition() - sf::Vector2f(maxQuantityEnteredSquareCollisions * offsetDuringRotation[i].x, maxQuantityEnteredSquareCollisions * offsetDuringRotation[i].y));
            }
        }
    }
};

class SkyFigure : public BaseFigureType
{
public:
    std::vector<std::vector<sf::Vector2f>> squaresOffsetPositionsDuringRotations =
    {
        {
            sf::Vector2f(2 * SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, 2 * SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(SQUARE_SIDE_LENGTH, 2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(-2 * SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(-2 * SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -2 * SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(2 * SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH)
        }
    };

    std::vector<sf::Vector2f> offsetDuringRotation =
    {
        sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
        sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, SQUARE_SIDE_LENGTH)
    };

    std::vector<Square> squares;
    int currentSquaresOffsetPositionsDuringRotations;
    int maxQuantityEnteredSquareCollisions;
    sf::Vector2f currentSquarePosition;

    SkyFigure() : currentSquarePosition(LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX) {
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 0), sf::Color(66, 170, 255)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 0), sf::Color(66, 170, 255)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(2, 0), sf::Color(66, 170, 255)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(3, 0), sf::Color(66, 170, 255)));
        currentSquaresOffsetPositionsDuringRotations = 3;
        maxQuantityEnteredSquareCollisions = 2;
    }

    std::vector<Square>& GetSquares() override { return squares; }
    sf::Vector2f GetCurrentPosition() override { return currentSquarePosition; }
    void SetCurrentPosition(sf::Vector2f newCurrentPosition) override { currentSquarePosition = newCurrentPosition; }

    bool MoveDown(std::vector<Square>& stoppedSquares) override {
        return MoveDownCommon(squares, currentSquarePosition, stoppedSquares);
    }

    bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) override {
        return CheckCollisionCommon(squares, stoppedSquares, offset);
    }

    void Spin(std::vector<Square>& stoppedSquares) override {
        SpinCommon(squares, stoppedSquares, squaresOffsetPositionsDuringRotations, currentSquaresOffsetPositionsDuringRotations, offsetDuringRotation, maxQuantityEnteredSquareCollisions);
    }

    bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) override {
        return CheckCollisionCommon(squares, stoppedSquares, sf::Vector2f(0, 0));
    }
};

class EmptyFigure : public BaseFigureType
{
public:
    std::vector<Square> squares;

    sf::Vector2f currentSquarePosition = sf::Vector2f(
        LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX
    );

    virtual std::vector<Square>& GetSquares() { return squares; }
    virtual sf::Vector2f GetCurrentPosition() { return currentSquarePosition; }
    virtual void SetCurrentPosition(sf::Vector2f newCurrentPosition) {}

    virtual bool MoveDown(std::vector<Square>& stoppedSquares) { return false; }
    virtual bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) { return false; }
    virtual void Spin(std::vector<Square>& stoppedSquares) {}
    virtual bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) { return false; }

    EmptyFigure() {
        squares = {};
    }

    ~EmptyFigure() {}
};

class YellowFigure : public BaseFigureType
{
public:
    std::vector<Square> squares;

    std::vector<std::vector<sf::Vector2f>> squaresOffsetPositionsDuringRotations =
    {
        {
            sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(0, SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, 0)
        },
        {
            sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(0, SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, 0)
        }
    };

    int currentSquaresOffsetPositionsDuringRotations;
    int maxQuantityEnteredSquareCollisions;

    std::vector<sf::Vector2f> offsetDuringRotation =
    {
        sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
        sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, SQUARE_SIDE_LENGTH)
    };

    sf::Vector2f currentSquarePosition = sf::Vector2f(
        LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX
    );

    YellowFigure()
    {
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 0), sf::Color::Yellow));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 0), sf::Color::Yellow));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 1), sf::Color::Yellow));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 1), sf::Color::Yellow));
        currentSquaresOffsetPositionsDuringRotations = 3;
        maxQuantityEnteredSquareCollisions = 0;
    }

    std::vector<Square>& GetSquares() override
    {
        return squares;
    }

    sf::Vector2f GetCurrentPosition() override
    {
        return currentSquarePosition;
    }

    void SetCurrentPosition(sf::Vector2f newCurrentPosition) override
    {
        currentSquarePosition = newCurrentPosition;
    }

    bool MoveDown(std::vector<Square>& stoppedSquares) override
    {
        return MoveDownCommon(squares, currentSquarePosition, stoppedSquares);
    }

    bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) override
    {
        return CheckCollisionCommon(squares, stoppedSquares, offset);
    }

    void Spin(std::vector<Square>& stoppedSquares) override
    {
        SpinCommon(squares, stoppedSquares, squaresOffsetPositionsDuringRotations, currentSquaresOffsetPositionsDuringRotations, offsetDuringRotation, maxQuantityEnteredSquareCollisions);
    }

    bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) override
    {
        return CheckCollisionCommon(squares, stoppedSquares, sf::Vector2f(0, 0));
    }
};

class PurpleFigure : public BaseFigureType
{
public:
    std::vector<Square> squares;

    std::vector<Square>& GetSquares() override {
        return squares;
    }

    sf::Vector2f GetCurrentPosition() override {
        return currentSquarePosition;
    }

    void SetCurrentPosition(sf::Vector2f newCurrentPosition) override {
        currentSquarePosition = newCurrentPosition;
    }

    std::vector<std::vector<sf::Vector2f>> squaresOffsetPositionsDuringRotations =
    {
        {
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH)
        }
    };

    int currentSquaresOffsetPositionsDuringRotations;
    int maxQuantityEnteredSquareCollisions;

    std::vector<sf::Vector2f> offsetDuringRotation =
    {
        sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
        sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, SQUARE_SIDE_LENGTH)
    };

    sf::Vector2f currentSquarePosition = sf::Vector2f(
        LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX
    );

    PurpleFigure() {
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 0), sf::Color(128, 0, 128)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 1), sf::Color(128, 0, 128)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 1), sf::Color(128, 0, 128)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(2, 1), sf::Color(128, 0, 128)));
        currentSquaresOffsetPositionsDuringRotations = 3;
        maxQuantityEnteredSquareCollisions = 1;
    }

    bool MoveDown(std::vector<Square>& stoppedSquares) override {
        return MoveDownCommon(squares, currentSquarePosition, stoppedSquares);
    }

    bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) override {
        return CheckCollisionCommon(squares, stoppedSquares, offset);
    }

    void Spin(std::vector<Square>& stoppedSquares) override {
        SpinCommon(squares, stoppedSquares, squaresOffsetPositionsDuringRotations, currentSquaresOffsetPositionsDuringRotations, offsetDuringRotation, maxQuantityEnteredSquareCollisions);
    }

    bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) override {
        return CheckCollisionCommon(squares, stoppedSquares, sf::Vector2f(0, 0));
    }
};

class BlueFigure : public BaseFigureType
{
public:
    std::vector<Square> squares;

    std::vector<Square>& GetSquares() override {
        return squares;
    }

    sf::Vector2f GetCurrentPosition() override {
        return currentSquarePosition;
    }

    void SetCurrentPosition(sf::Vector2f newCurrentPosition) override {
        currentSquarePosition = newCurrentPosition;
    }

    std::vector<std::vector<sf::Vector2f>> squaresOffsetPositionsDuringRotations =
    {
        {
            sf::Vector2f(2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(0, 2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(-2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(0, -2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH)
        }
    };

    int currentSquaresOffsetPositionsDuringRotations;
    int maxQuantityEnteredSquareCollisions;

    std::vector<sf::Vector2f> offsetDuringRotation =
    {
        sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
        sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, SQUARE_SIDE_LENGTH)
    };

    sf::Vector2f currentSquarePosition = sf::Vector2f(
        LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX
    );

    BlueFigure() {
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 0), sf::Color::Blue));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 1), sf::Color::Blue));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 1), sf::Color::Blue));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(2, 1), sf::Color::Blue));
        currentSquaresOffsetPositionsDuringRotations = 3;
        maxQuantityEnteredSquareCollisions = 1;
    }

    bool MoveDown(std::vector<Square>& stoppedSquares) override {
        return MoveDownCommon(squares, currentSquarePosition, stoppedSquares);
    }

    bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) override {
        return CheckCollisionCommon(squares, stoppedSquares, offset);
    }

    void Spin(std::vector<Square>& stoppedSquares) override {
        SpinCommon(squares, stoppedSquares, squaresOffsetPositionsDuringRotations, currentSquaresOffsetPositionsDuringRotations, offsetDuringRotation, maxQuantityEnteredSquareCollisions);
    }

    bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) override {
        return CheckCollisionCommon(squares, stoppedSquares, sf::Vector2f(0, 0));
    }
};

class OrangeFigure : public BaseFigureType
{
public:
    std::vector<Square> squares;

    std::vector<Square>& GetSquares() override {
        return squares;
    }

    sf::Vector2f GetCurrentPosition() override {
        return currentSquarePosition;
    }

    void SetCurrentPosition(sf::Vector2f newCurrentPosition) override {
        currentSquarePosition = newCurrentPosition;
    }

    std::vector<std::vector<sf::Vector2f>> squaresOffsetPositionsDuringRotations =
    {
        {
            sf::Vector2f(0, 2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
        },
        {
            sf::Vector2f(-2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
        },
        {
            sf::Vector2f(0, -2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
        },
        {
            sf::Vector2f(2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
        }
    };

    int currentSquaresOffsetPositionsDuringRotations;
    int maxQuantityEnteredSquareCollisions;

    std::vector<sf::Vector2f> offsetDuringRotation =
    {
        sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
        sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, SQUARE_SIDE_LENGTH)
    };

    sf::Vector2f currentSquarePosition = sf::Vector2f(
        LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX
    );

    OrangeFigure() {
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(2, 0), sf::Color(255, 165, 0)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 1), sf::Color(255, 165, 0)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 1), sf::Color(255, 165, 0)));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(2, 1), sf::Color(255, 165, 0)));
        currentSquaresOffsetPositionsDuringRotations = 3;
        maxQuantityEnteredSquareCollisions = 1;
    }

    bool MoveDown(std::vector<Square>& stoppedSquares) override {
        return MoveDownCommon(squares, currentSquarePosition, stoppedSquares);
    }

    bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) override {
        return CheckCollisionCommon(squares, stoppedSquares, offset);
    }

    void Spin(std::vector<Square>& stoppedSquares) override {
        SpinCommon(squares, stoppedSquares, squaresOffsetPositionsDuringRotations, currentSquaresOffsetPositionsDuringRotations, offsetDuringRotation, maxQuantityEnteredSquareCollisions);
    }

    bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) override {
        return CheckCollisionCommon(squares, stoppedSquares, sf::Vector2f(0, 0));
    }
};

class GreenFigure : public BaseFigureType
{
public:
    std::vector<std::vector<sf::Vector2f>> squaresOffsetPositionsDuringRotations =
    {
        {
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0)
        },
        {
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(-2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0)
        },
        {
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, -2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0)
        },
        {
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0)
        }
    };

    std::vector<sf::Vector2f> offsetDuringRotation =
    {
        sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
        sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, SQUARE_SIDE_LENGTH)
    };

    std::vector<Square> squares;
    int currentSquaresOffsetPositionsDuringRotations;
    int maxQuantityEnteredSquareCollisions;
    sf::Vector2f currentSquarePosition;

    GreenFigure() : currentSquarePosition(LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX)
    {
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 0), sf::Color::Green));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(2, 0), sf::Color::Green));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 1), sf::Color::Green));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 1), sf::Color::Green));
        currentSquaresOffsetPositionsDuringRotations = 3;
        maxQuantityEnteredSquareCollisions = 1;
    }

    std::vector<Square>& GetSquares() override { return squares; }
    sf::Vector2f GetCurrentPosition() override { return currentSquarePosition; }
    void SetCurrentPosition(sf::Vector2f newCurrentPosition) override { currentSquarePosition = newCurrentPosition; }

    bool MoveDown(std::vector<Square>& stoppedSquares) override {
        return MoveDownCommon(squares, currentSquarePosition, stoppedSquares);
    }

    bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) override {
        return CheckCollisionCommon(squares, stoppedSquares, offset);
    }

    void Spin(std::vector<Square>& stoppedSquares) override {
        SpinCommon(squares, stoppedSquares, squaresOffsetPositionsDuringRotations, currentSquaresOffsetPositionsDuringRotations, offsetDuringRotation, maxQuantityEnteredSquareCollisions);
    }

    bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) override {
        return CheckCollisionCommon(squares, stoppedSquares, sf::Vector2f(0, 0));
    }
};

class RedFigure : public BaseFigureType
{
public:
    std::vector<Square> squares;

    std::vector<Square>& GetSquares() override {
        return squares;
    }

    sf::Vector2f GetCurrentPosition() override {
        return currentSquarePosition;
    }

    void SetCurrentPosition(sf::Vector2f newCurrentPosition) override {
        currentSquarePosition = newCurrentPosition;
    }

    std::vector<std::vector<sf::Vector2f>> squaresOffsetPositionsDuringRotations =
    {
        {
            sf::Vector2f(2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(0, 2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(-2 * SQUARE_SIDE_LENGTH, 0),
            sf::Vector2f(-SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH)
        },
        {
            sf::Vector2f(0, -2 * SQUARE_SIDE_LENGTH),
            sf::Vector2f(SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH),
            sf::Vector2f(0, 0),
            sf::Vector2f(SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH)
        }
    };

    int currentSquaresOffsetPositionsDuringRotations;
    int maxQuantityEnteredSquareCollisions;

    std::vector<sf::Vector2f> offsetDuringRotation =
    {
        sf::Vector2f(-SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, -SQUARE_SIDE_LENGTH),
        sf::Vector2f(SQUARE_SIDE_LENGTH, 0),
        sf::Vector2f(0, SQUARE_SIDE_LENGTH)
    };

    sf::Vector2f currentSquarePosition = sf::Vector2f(
        LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 3 * SQUARE_SIDE_LENGTH,
        TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX
    );

    RedFigure() {
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(0, 0), sf::Color::Red));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 0), sf::Color::Red));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(1, 1), sf::Color::Red));
        squares.push_back(Square(currentSquarePosition, sf::Vector2f(2, 1), sf::Color::Red));
        currentSquaresOffsetPositionsDuringRotations = 3;
        maxQuantityEnteredSquareCollisions = 1;
    }

    bool MoveDown(std::vector<Square>& stoppedSquares) override {
        return MoveDownCommon(squares, currentSquarePosition, stoppedSquares);
    }

    bool CheckCollision(std::vector<Square>& stoppedSquares, sf::Vector2f offset) override {
        return CheckCollisionCommon(squares, stoppedSquares, offset);
    }

    void Spin(std::vector<Square>& stoppedSquares) override {
        SpinCommon(squares, stoppedSquares, squaresOffsetPositionsDuringRotations, currentSquaresOffsetPositionsDuringRotations, offsetDuringRotation, maxQuantityEnteredSquareCollisions);
    }

    bool CheckCollisionForSpin(std::vector<Square>& squares, std::vector<Square>& stoppedSquares) override {
        return CheckCollisionCommon(squares, stoppedSquares, sf::Vector2f(0, 0));
    }
};

class SquareMatrix
{
public:
    std::vector<Square> squares;

    SquareMatrix(sf::Vector2f playingAreaPosition) {
        for (int i = 0; i < SQUARES_QUANTITY_HORIZONTALLY; i++) {
            for (int j = 0; j < SQUARES_QUANTITY_VERTICALLY; j++) {
                squares.push_back(Square(sf::Vector2f(LEFT_OFFSET_FOR_SQUARE_MATRIX, TOP_OFFSET_FOR_SQUARE_MATRIX) + playingAreaPosition, sf::Vector2f(i, j), sf::Color::White));
            }
        }
    }
};

class StoppedSquares
{
public:
    std::vector<Square> squares;

    StoppedSquares() {
        squares = {};
    }
};

class PhantomFigure {
public:
    std::vector<Square> squares;

    BaseFigureType* figure;
    StoppedSquares* stoppedSquares;

    PhantomFigure(BaseFigureType* baseFigure, StoppedSquares* baseStoppedSquares) {
        figure = baseFigure;
        squares = figure->GetSquares();
        for (int i = 0; i < squares.size(); i++) {
            squares[i].setFillColor(sf::Color(210, 210, 210));
        }
        stoppedSquares = baseStoppedSquares;
    }

    void UpdatePosition() {
        sf::Vector2f offset = sf::Vector2f(0, SQUARE_SIDE_LENGTH);
        bool flag = true;
        while (flag) {
            for (int i = 0; i < figure->GetSquares().size(); i++) {
                for (int j = 0; j < stoppedSquares->squares.size(); j++) {
                    if (figure->GetSquares()[i].getPosition() + offset == stoppedSquares->squares[j].getPosition()) {
                        flag = false;
                    }
                }
                if (figure->GetSquares()[i].getPosition().y + offset.y == TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX + 20 * SQUARE_SIDE_LENGTH) {
                    flag = false;
                }
            }
            if (!flag) {
                for (int i = 0; i < squares.size(); i++) {
                    squares[i].setPosition(figure->GetSquares()[i].getPosition() + offset - sf::Vector2f(0, SQUARE_SIDE_LENGTH));
                }
            }
            offset += sf::Vector2f(0, SQUARE_SIDE_LENGTH);
        }
    }
};

class PlayingArea: public sf::RectangleShape
{
public:
    float width;
    float height;
    SquareMatrix* squareMatrix;
    BaseFigureType* currentFigure;
    StoppedSquares* stoppedSquares;
    PhantomFigure* phantomFigure;
    bool isDeath;
    BaseFigureType* pocketFigure;
    std::vector<BaseFigureType*> nextFigures;
    std::vector<BaseFigureType*> nextFourFigures;

    PlayingArea() : sf::RectangleShape(sf::Vector2f(0, 0)) {
        width = (SQUARES_QUANTITY_HORIZONTALLY + 2) * SQUARE_SIDE_LENGTH;
        height = (SQUARES_QUANTITY_VERTICALLY + 2) * SQUARE_SIDE_LENGTH;
        setSize(sf::Vector2f(width, height));
        setPosition(LEFT_OFFSET_FOR_PLAYING_AREA, TOP_OFFSET_FOR_PLAYING_AREA);
        setFillColor(sf::Color(180, 180, 180));
        squareMatrix = new SquareMatrix(this->getPosition());
        nextFigures = {};
        FillInNextFigures();
        nextFourFigures = {};
        FillInNextFourFigures();
        currentFigure = getFigure();
        stoppedSquares = new StoppedSquares();
        phantomFigure = new PhantomFigure(currentFigure, stoppedSquares);
        phantomFigure->UpdatePosition();
        isDeath = false;
        pocketFigure = new EmptyFigure();
    }

    void FillInNextFourFigures() {
        nextFourFigures = {};
        for (int i = nextFigures.size() - 1; i > nextFigures.size() - 5; i--) {
            nextFourFigures.push_back(nextFigures[i]);
        }

        for (int i = 0; i < nextFourFigures.size(); i++) {
            float minXPosition = nextFourFigures[i]->GetSquares()[0].getPosition().x;
            float minYPosition = nextFourFigures[i]->GetSquares()[0].getPosition().y;
            for (int j = 0; j < nextFourFigures[i]->GetSquares().size(); j++) {
                if (nextFourFigures[i]->GetSquares()[j].getPosition().x < minXPosition) {
                    minXPosition = nextFourFigures[i]->GetSquares()[j].getPosition().x;
                }
                if (nextFourFigures[i]->GetSquares()[j].getPosition().y < minYPosition) {
                    minYPosition = nextFourFigures[i]->GetSquares()[j].getPosition().y;
                }
            }

            float deltaX = LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX - minXPosition;
            float deltaY = TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX - minYPosition;

            for (int j = 0; j < nextFourFigures[i]->GetSquares().size(); j++) {
                nextFourFigures[i]->GetSquares()[j].setPosition(nextFourFigures[i]->GetSquares()[j].getPosition() + sf::Vector2f(deltaX, deltaY));
            }
            for (int j = 0; j < nextFourFigures[i]->GetSquares().size(); j++) {
                nextFourFigures[i]->GetSquares()[j].setPosition(nextFourFigures[i]->GetSquares()[j].getPosition() + sf::Vector2f(14 * SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH + i * 5 * SQUARE_SIDE_LENGTH));
            }
        }
    }

    void FillInNextFigures() {
        std::random_device rd;
        std::mt19937 g(rd());

        std::vector<BaseFigureType*> secondVectorNextFigures = {};

        secondVectorNextFigures.push_back(new SkyFigure());
        secondVectorNextFigures.push_back(new YellowFigure());
        secondVectorNextFigures.push_back(new PurpleFigure());
        secondVectorNextFigures.push_back(new BlueFigure());
        secondVectorNextFigures.push_back(new OrangeFigure());
        secondVectorNextFigures.push_back(new GreenFigure());
        secondVectorNextFigures.push_back(new RedFigure());

        shuffle(secondVectorNextFigures.begin(), secondVectorNextFigures.end(), g);

        for (int i = 0; i < nextFigures.size(); i++) {
            secondVectorNextFigures.push_back(nextFigures[i]);
        }

        nextFigures = secondVectorNextFigures;
    }

    BaseFigureType* getFigure() {
        if (nextFigures.size() == 7) {
            FillInNextFigures();
        }
        FillInNextFourFigures();
        BaseFigureType* nextFigure = nextFigures.back();
        for (int i = 0; i < nextFigure->GetSquares().size(); i++) {
            nextFigure->GetSquares()[i].setPosition(nextFigure->GetSquares()[i].getPosition() + sf::Vector2f(-11 * SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH));
        }
        nextFigures.pop_back();
        FillInNextFourFigures();
        return nextFigure;
    }

    void ChangePocketFigure() {
        if (dynamic_cast<EmptyFigure*>(pocketFigure) != nullptr) {
            pocketFigure = currentFigure;

            pocketFigure;

            float minXPosition = pocketFigure->GetSquares()[0].getPosition().x;
            float minYPosition = pocketFigure->GetSquares()[0].getPosition().y;
            for (int i = 0; i < pocketFigure->GetSquares().size(); i++) {
                if (pocketFigure->GetSquares()[i].getPosition().x < minXPosition) {
                    minXPosition = pocketFigure->GetSquares()[i].getPosition().x;
                }
                if (pocketFigure->GetSquares()[i].getPosition().y < minYPosition) {
                    minYPosition = pocketFigure->GetSquares()[i].getPosition().y;
                }
            }

            float deltaX = LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX - minXPosition;
            float deltaY = TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX - minYPosition;

            for (int i = 0; i < pocketFigure->GetSquares().size(); i++) {
                pocketFigure->GetSquares()[i].setPosition(pocketFigure->GetSquares()[i].getPosition() + sf::Vector2f(deltaX, deltaY));
            }
            for (int i = 0; i < pocketFigure->GetSquares().size(); i++) {
                pocketFigure->GetSquares()[i].setPosition(pocketFigure->GetSquares()[i].getPosition() + sf::Vector2f(-8 * SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH));
            }
            currentFigure = getFigure();
            phantomFigure = new PhantomFigure(currentFigure, stoppedSquares);
            phantomFigure->UpdatePosition();
        }
        else {
            BaseFigureType* helpPocketFigure = currentFigure;

            currentFigure = pocketFigure;
            for (int i = 0; i < currentFigure->GetSquares().size(); i++) {
                currentFigure->GetSquares()[i].setPosition(currentFigure->GetSquares()[i].getPosition() + sf::Vector2f(11 * SQUARE_SIDE_LENGTH, -SQUARE_SIDE_LENGTH));
            }
            phantomFigure = new PhantomFigure(currentFigure, stoppedSquares);
            phantomFigure->UpdatePosition();

            pocketFigure = helpPocketFigure;
            float minXPosition = pocketFigure->GetSquares()[0].getPosition().x;
            float minYPosition = pocketFigure->GetSquares()[0].getPosition().y;
            for (int i = 0; i < pocketFigure->GetSquares().size(); i++) {
                if (pocketFigure->GetSquares()[i].getPosition().x < minXPosition) {
                    minXPosition = pocketFigure->GetSquares()[i].getPosition().x;
                }
                if (pocketFigure->GetSquares()[i].getPosition().y < minYPosition) {
                    minYPosition = pocketFigure->GetSquares()[i].getPosition().y;
                }
            }

            float deltaX = LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX - minXPosition;
            float deltaY = TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX - minYPosition;

            for (int i = 0; i < pocketFigure->GetSquares().size(); i++) {
                pocketFigure->GetSquares()[i].setPosition(pocketFigure->GetSquares()[i].getPosition() + sf::Vector2f(deltaX, deltaY));
            }
            for (int i = 0; i < pocketFigure->GetSquares().size(); i++) {
                pocketFigure->GetSquares()[i].setPosition(pocketFigure->GetSquares()[i].getPosition() + sf::Vector2f(-8 * SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH));
            }
        }
        FillInNextFourFigures();
    }

    bool GetNextFigure() {
        for (int i = 0; i < currentFigure->GetSquares().size(); i++) {
            stoppedSquares->squares.push_back(currentFigure->GetSquares()[i]);
        }
        DestroyLines();
        currentFigure = getFigure();
        CheckDeath();
        phantomFigure = new PhantomFigure(currentFigure, stoppedSquares);
        phantomFigure->UpdatePosition();
        return false;
    }

    bool MoveDownCurrentFigure() {
        return currentFigure->MoveDown(stoppedSquares->squares);
    }

    void DestroyLines() {
        float minYPosition = currentFigure->GetSquares()[0].getPosition().y;
        float maxYPosition = currentFigure->GetSquares()[0].getPosition().y;

        for (int i = 0; i < currentFigure->GetSquares().size(); i++) {
            if (currentFigure->GetSquares()[i].getPosition().y < minYPosition) {
                minYPosition = currentFigure->GetSquares()[i].getPosition().y;
            }
            else if (currentFigure->GetSquares()[i].getPosition().y > maxYPosition) {
                maxYPosition = currentFigure->GetSquares()[i].getPosition().y;
            }
        }

        int stoppedSquareOnLineQuantity;
        for (int i = minYPosition; i < maxYPosition + SQUARE_SIDE_LENGTH; i += SQUARE_SIDE_LENGTH) {
            stoppedSquareOnLineQuantity = 0;
            for (int j = LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX; j < LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 10 * SQUARE_SIDE_LENGTH; j += SQUARE_SIDE_LENGTH) {
                for (int k = 0; k < stoppedSquares->squares.size(); k++) {
                    if (stoppedSquares->squares[k].getPosition() == sf::Vector2f(j, i)) {
                        stoppedSquareOnLineQuantity++;
                        break;
                    }
                }
            }
            if (stoppedSquareOnLineQuantity == 10) {
                for (int j = 0; j < stoppedSquares->squares.size(); j++) {
                    if (stoppedSquares->squares[j].getPosition().y == i) {
                        stoppedSquares->squares.erase(stoppedSquares->squares.begin() + j);
                        j--;
                    }
                }
                for (int j = 0; j < stoppedSquares->squares.size(); j++) {
                    if (stoppedSquares->squares[j].getPosition().y < i) {
                        stoppedSquares->squares[j].setPosition(stoppedSquares->squares[j].getPosition() + sf::Vector2f(0, SQUARE_SIDE_LENGTH));
                    }
                }
            }
        }
    }

    void CheckDeath() {
        for (int i = 0; i < currentFigure->GetSquares().size(); i++) {
            for (int j = 0; j < stoppedSquares->squares.size(); j++) {
                if (currentFigure->GetSquares()[i].getPosition() == stoppedSquares->squares[j].getPosition()) {
                    isDeath = true;
                }
            }
        }
    }
};

class Controller
{
public:
    Controller() {
    }
};

int main()
{
    std::srand(std::time(nullptr));

    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(desktopMode.width, desktopMode.height), L"Tetris", sf::Style::Default);

    sf::Vector2u size = window.getSize();
    int windowWidth = size.x;
    int windowHeight = size.y;

    window.setVerticalSyncEnabled(true);

    PlayingArea* playingArea = new PlayingArea();

    sf::RectangleShape pocketBackground(sf::Vector2f(8 * SQUARE_SIDE_LENGTH, 8 * SQUARE_SIDE_LENGTH));
    pocketBackground.setPosition(LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX - 10 * SQUARE_SIDE_LENGTH, TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX - SQUARE_SIDE_LENGTH);
    pocketBackground.setFillColor(sf::Color(180, 180, 180));

    std::vector<Square> pocketMatrix;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            pocketMatrix.push_back(Square(sf::Vector2f(LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX - 9 * SQUARE_SIDE_LENGTH, TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX), sf::Vector2f(i, j), sf::Color::White));
        }
    }

    sf::RectangleShape nextFiguresBackground(sf::Vector2f(8 * SQUARE_SIDE_LENGTH, 21 * SQUARE_SIDE_LENGTH));
    nextFiguresBackground.setPosition(LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 12 * SQUARE_SIDE_LENGTH, TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX - SQUARE_SIDE_LENGTH);
    nextFiguresBackground.setFillColor(sf::Color(180, 180, 180));

    std::vector<sf::RectangleShape> backgrounLines = {
        sf::RectangleShape(sf::Vector2f(8 * SQUARE_SIDE_LENGTH, 1 * SQUARE_SIDE_LENGTH)),
        sf::RectangleShape(sf::Vector2f(8 * SQUARE_SIDE_LENGTH, 1 * SQUARE_SIDE_LENGTH)),
        sf::RectangleShape(sf::Vector2f(8 * SQUARE_SIDE_LENGTH, 1 * SQUARE_SIDE_LENGTH))
    };
    for (int i = 0; i < backgrounLines.size(); i++) {
        backgrounLines[i].setPosition(LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 12 * SQUARE_SIDE_LENGTH, TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX + 4 * SQUARE_SIDE_LENGTH + 5 * i * SQUARE_SIDE_LENGTH);
        backgrounLines[i].setFillColor(sf::Color(180, 180, 180));
    }

    std::vector<Square> nextFiguresMatrix;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 19; j++) {
            nextFiguresMatrix.push_back(Square(sf::Vector2f(LEFT_OFFSET_FOR_PLAYING_AREA + LEFT_OFFSET_FOR_SQUARE_MATRIX + 13 * SQUARE_SIDE_LENGTH, TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX), sf::Vector2f(i, j), sf::Color::White));
        }
    }

    int isKeyDownPressedCount = 0;
    int isKeyRightPressedCount = 0;
    int isKeyLeftPressedCount = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                size = window.getSize();
                windowWidth = size.x;
                windowHeight = size.y;
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
                    playingArea->ChangePocketFigure();
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    sf::Vector2f offset = sf::Vector2f(0, SQUARE_SIDE_LENGTH);
                    bool flag = true;
                    while (flag) {
                        for (int i = 0; i < playingArea->currentFigure->GetSquares().size(); i++) {
                            for (int j = 0; j < playingArea->stoppedSquares->squares.size(); j++) {
                                if (playingArea->currentFigure->GetSquares()[i].getPosition() + offset == playingArea->stoppedSquares->squares[j].getPosition()) {
                                    flag = false;
                                }
                            }
                            if (playingArea->currentFigure->GetSquares()[i].getPosition().y + offset.y == TOP_OFFSET_FOR_PLAYING_AREA + TOP_OFFSET_FOR_SQUARE_MATRIX + 20 * SQUARE_SIDE_LENGTH) {
                                flag = false;
                            }
                        }
                        if (!flag) {
                            for (int i = 0; i < playingArea->currentFigure->GetSquares().size(); i++) {
                                playingArea->currentFigure->GetSquares()[i].setPosition(playingArea->currentFigure->GetSquares()[i].getPosition() + offset - sf::Vector2f(0, SQUARE_SIDE_LENGTH));
                            }
                            playingArea->GetNextFigure();
                        }
                        offset += sf::Vector2f(0, SQUARE_SIDE_LENGTH);
                    }
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                    playingArea->currentFigure->Spin(playingArea->stoppedSquares->squares);
                    playingArea->currentFigure->SetCurrentPosition(playingArea->currentFigure->GetSquares()[0].getPosition());
                    playingArea->phantomFigure->UpdatePosition();
                }
            }
        }

        isKeyDownPressedCount++;
        isKeyRightPressedCount++;
        isKeyLeftPressedCount++;

        if (isKeyDownPressedCount > 2 && sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            sf::Vector2f offset = sf::Vector2f(0, SQUARE_SIDE_LENGTH);
            if (!playingArea->currentFigure->CheckCollision(playingArea->stoppedSquares->squares, offset)) {
                for (int i = 0; i < playingArea->currentFigure->GetSquares().size(); i++) {
                    playingArea->currentFigure->GetSquares()[i].setPosition(playingArea->currentFigure->GetSquares()[i].getPosition() + offset);
                }
                playingArea->currentFigure->SetCurrentPosition(playingArea->currentFigure->GetSquares()[0].getPosition());
            }
            isKeyDownPressedCount = 0;
        }

        if (isKeyRightPressedCount > 6 && sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            sf::Vector2f offset = sf::Vector2f(SQUARE_SIDE_LENGTH, 0);
            if (!playingArea->currentFigure->CheckCollision(playingArea->stoppedSquares->squares, offset)) {
                for (int i = 0; i < playingArea->currentFigure->GetSquares().size(); i++) {
                    playingArea->currentFigure->GetSquares()[i].setPosition(playingArea->currentFigure->GetSquares()[i].getPosition() + offset);
                }
                playingArea->currentFigure->SetCurrentPosition(playingArea->currentFigure->GetSquares()[0].getPosition());
            }
            playingArea->phantomFigure->UpdatePosition();
            isKeyRightPressedCount = 0;
        }

        else if (isKeyLeftPressedCount > 6 && sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            sf::Vector2f offset = sf::Vector2f(-SQUARE_SIDE_LENGTH, 0);
            if (!playingArea->currentFigure->CheckCollision(playingArea->stoppedSquares->squares, offset)) {
                for (int i = 0; i < playingArea->currentFigure->GetSquares().size(); i++) {
                    playingArea->currentFigure->GetSquares()[i].setPosition(playingArea->currentFigure->GetSquares()[i].getPosition() + offset);
                }
                playingArea->currentFigure->SetCurrentPosition(playingArea->currentFigure->GetSquares()[0].getPosition());
            }
            playingArea->phantomFigure->UpdatePosition();
            isKeyLeftPressedCount = 0;
        }
        
        window.clear(sf::Color::Black);

        window.draw(*playingArea);

        for (int i = 0; i < playingArea->squareMatrix->squares.size(); i++) {
            window.draw(playingArea->squareMatrix->squares[i]);
        }

        for (int i = 0; i < playingArea->phantomFigure->squares.size(); i++) {
            window.draw(playingArea->phantomFigure->squares[i]);
        }

        for (int i = 0; i < playingArea->currentFigure->GetSquares().size(); i++) {
            window.draw(playingArea->currentFigure->GetSquares()[i]);
        }

        for (int i = 0; i < playingArea->stoppedSquares->squares.size(); i++) {
            window.draw(playingArea->stoppedSquares->squares[i]);
        }

        window.draw(nextFiguresBackground);
        for (int i = 0; i < nextFiguresMatrix.size(); i++) {
            window.draw(nextFiguresMatrix[i]);
        }

        for (int i = 0; i < backgrounLines.size(); i++) {
            window.draw(backgrounLines[i]);
        }

        for (int i = 0; i < playingArea->nextFourFigures.size(); i++) {
            for (int j = 0; j < playingArea->nextFourFigures[i]->GetSquares().size(); j++) {
                window.draw(playingArea->nextFourFigures[i]->GetSquares()[j]);
            }
        }

        window.draw(pocketBackground);
        for (int i = 0; i < pocketMatrix.size(); i++) {
            window.draw(pocketMatrix[i]);
        }

        for (int i = 0; i < playingArea->pocketFigure->GetSquares().size(); i++) {
            window.draw(playingArea->pocketFigure->GetSquares()[i]);
        }

        window.display();

        if (playingArea->MoveDownCurrentFigure()) {
            playingArea->GetNextFigure();
        }

        if (playingArea->isDeath) {
            break;
        }
    }
}
