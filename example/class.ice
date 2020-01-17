@Animal() {
    @yep(), println("I can't yep!");

    return @() {};
}

@Cat() {
    @this: Animal();

    this.yep: @(), println("Mew!");

    return this;
}

@Pig() {
    @this: Animal();

    this.yep: @(), println("Heng heng!");
    this.weight: 100;

    return this;
}

pig: Pig();
cat: Cat();

cat.yep();
pig.yep();
