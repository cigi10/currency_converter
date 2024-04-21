# Made using Python and Turtle
import turtle

# Window setup
wn = turtle.Screen()
wn.title("Pong")
wn.bgcolor("#0b033d")
wn.setup(width=800, height=600)
wn.tracer(0)

# Score variables
score_a = 0
score_b = 0

# Paddle A setup
paddle_a = turtle.Turtle()
paddle_a.speed(0)
paddle_a.shape("square")
paddle_a.color("#b3d5ff")
paddle_a.shapesize(stretch_wid=5, stretch_len=1)
paddle_a.penup()
paddle_a.goto(-350, 0)

# Paddle B setup
paddle_b = turtle.Turtle()
paddle_b.speed(0)
paddle_b.shape("square")
paddle_b.color("#d5ffad")
paddle_b.shapesize(stretch_wid=5, stretch_len=1)
paddle_b.penup()
paddle_b.goto(350, 0)

# Ball setup
ball = turtle.Turtle()
ball.speed(0)
ball.shape("circle")
ball.color("#fcba03")
ball.penup()
ball.goto(0, 0)
ball.dx = 2
ball.dy = 2

# Pen setup
pen = turtle.Turtle()
pen.speed(0)
pen.shape("square")
pen.color("white")
pen.penup()
pen.hideturtle()
pen.goto(0, 260)
pen.write("Player A: 0  Player B: 0", align="center", font=("Trebuchet MS", 24, "normal"))

# Functions for paddle movement
def paddle_a_up():
    y = paddle_a.ycor()
    y += 20
    paddle_a.sety(y)

def paddle_a_down():
    y = paddle_a.ycor()
    y -= 20
    paddle_a.sety(y)

def paddle_b_up():
    y = paddle_b.ycor()
    y += 20
    paddle_b.sety(y)

def paddle_b_down():
    y = paddle_b.ycor()
    y -= 20
    paddle_b.sety(y)

# Keyboard bindings for paddle movement
wn.listen()
wn.onkeypress(paddle_a_up, "w")
wn.onkeypress(paddle_a_down, "s")
wn.onkeypress(paddle_b_up, "Up")
wn.onkeypress(paddle_b_down, "Down")

# Main game loop function
def move():
    global score_a, score_b

    # Move the ball
    ball.setx(ball.xcor() + ball.dx)
    ball.sety(ball.ycor() + ball.dy)

    # Border checking
    if ball.ycor() > 290 or ball.ycor() < -290:
        ball.dy *= -1

    if ball.xcor() > 350:
        score_a += 1
        pen.clear()
        pen.write("Player A: {}  Player B: {}".format(score_a, score_b), align="center", font=("Courier", 24, "normal"))
        ball.goto(0, 0)
        ball.dx *= -1

    if ball.xcor() < -350:
        score_b += 1
        pen.clear()
        pen.write("Player A: {}  Player B: {}".format(score_a, score_b), align="center", font=("Courier", 24, "normal"))
        ball.goto(0, 0)
        ball.dx *= -1

    # Paddle and ball collisions
    if (ball.dx > 0 and paddle_b.xcor() - 20 < ball.xcor() < paddle_b.xcor() + 20) and (paddle_b.ycor() - 50 < ball.ycor() < paddle_b.ycor() + 50):
        ball.dx *= -1

    if (ball.dx < 0 and paddle_a.xcor() - 20 < ball.xcor() < paddle_a.xcor() + 20) and (paddle_a.ycor() - 50 < ball.ycor() < paddle_a.ycor() + 50):
        ball.dx *= -1

    # Update the screen
    wn.update()

    # Check if either player has won by reaching a score difference of 5
    if abs(score_a - score_b) >= 5:
        wn.bye()  # Close the window if the game ends
    else:
        wn.ontimer(move, 10)  # Continue the game loop

# Start the game loop
move()

# Keep the window open
wn.mainloop()
