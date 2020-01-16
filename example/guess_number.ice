println("Guess the number!");

serect_number: time() % 100;

do
{
    guess_number: eval(input());
    if guess_number = serect_number,
        break;
    elif guess_number < serect_number,
        println("Too less!");
    else,
        println("Too big!");
} while true;

println("Guess right!");
