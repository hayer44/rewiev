#!/usr/bin/env python2

from collections import deque
from random import choice
from random import randint

RANDOM_CODE = 'random'


def sum_pair(a, b):
    return a[0] + b[0], a[1] + b[1]


class Unit(object):
    MAX_HUNGER = 20
    MIN_HUNGER = 0
    PREGNANT_TIME = 100
    MIN_PREGNANT_TIME = 0
    HUNGER_TRESHOLD = MAX_HUNGER * 3 / 5

    def __init__(self, unit_type, coord):
        self.unit_type = unit_type
        self.coord = coord
        self.feed()
        self.after_birth()
        self.id_ = 0

    def feed(self):
        self.last_feed = 0

    def after_birth(self):
        self.last_birth = 0


class Game(object):
    predator_code = 'predator'
    prey_code = 'prey'
    barrier_code = 'barrier'
    random_code = 'random'
    unit_types = [predator_code, prey_code, barrier_code]
    empty_place_code = 0
    left = (-1, 0)
    right = (1, 0)
    up = (0, 1)
    down = (0, -1)
    same_place = (0, 0)
    min_coord = 0

    def __init__(self, field_size):
        self.history = []
        self.predators_amount = 0
        self.smb_is_living = True
        self.preys_amount = 0
        self.last_id_ = 0
        self.queue = deque()
        self.still_living = dict()

        self.field = []
        self.field_size = field_size
        for i in xrange(field_size):
            row = [Game.empty_place_code] * field_size
            self.field.append(row)

    def add_unit(self, unit_type=RANDOM_CODE, coord=RANDOM_CODE, random_age=False):
        if unit_type == self.random_code:
            unit_type = choice(self.unit_types)

        if coord == Game.random_code:
            coord = self.find_empty_random_place()

        unit = Unit(unit_type, coord)
        if random_age:
            unit.last_birth = randint(Unit.MIN_PREGNANT_TIME, Unit.PREGNANT_TIME)
            unit.last_feed = randint(Unit.MIN_HUNGER, Unit.MAX_HUNGER)

        self.last_id_ += 1
        unit.id_ = self.last_id_

        if unit_type != Game.barrier_code:
            self.queue.append(unit.id_)

        if unit_type == Game.predator_code:
            self.predators_amount += 1

        if unit_type == Game.prey_code:
            self.preys_amount += 1

        self.still_living[self.last_id_] = unit
        self.field[coord[1]][coord[0]] = self.last_id_

    def find_empty_random_place(self):
        while True:
            x = randint(Game.min_coord, self.field_size - 1)
            y = randint(Game.min_coord, self.field_size - 1)
            if self.field[y][x] == Game.empty_place_code:
                break
        return (x, y)

    def output(self):
        for y in xrange(self.field_size):
            for x in xrange(self.field_size):
                if (self.field[y][x] == Game.empty_place_code):
                    print '\'',
                else:
                    if (self.still_living[self.field[y][x]].unit_type == Game.predator_code):
                        print '@',
                    if (self.still_living[self.field[y][x]].unit_type == Game.prey_code):
                        print '%',
                    if (self.still_living[self.field[y][x]].unit_type == Game.barrier_code):
                        print '#',
            print

    def empty_places_around(self, coord):
        x = coord[0]
        y = coord[1]
        places = []
        if x + 1 < self.field_size:
            if self.field[y][x + 1] == Game.empty_place_code:
                places.append(Game.right)

        if x - 1 >= self.min_coord:
            if self.field[y][x - 1] == Game.empty_place_code:
                places.append(Game.left)

        if y + 1 < self.field_size:
            if self.field[y + 1][x] == Game.empty_place_code:
                places.append(Game.up)

        if y - 1 >= self.min_coord:
            if self.field[y - 1][x] == Game.empty_place_code:
                places.append(Game.down)

        return places

    def preys_around(self, coord):
        x = coord[0]
        y = coord[1]
        preys = []
        if x + 1 < self.field_size:
            if self.is_living(self.field[y][x + 1]):
                if self.still_living.get(self.field[y][x + 1]).unit_type == Game.prey_code:
                    preys.append(self.field[y][x + 1])

        if y + 1 < self.field_size:
            if self.is_living(self.field[y + 1][x]):
                if self.still_living.get(self.field[y + 1][x]).unit_type == Game.prey_code:
                    preys.append(self.field[y + 1][x])

        if x - 1 >= self.min_coord:
            if self.is_living(self.field[y][x - 1]):
                if self.still_living.get(self.field[y][x - 1]).unit_type == Game.prey_code:
                    preys.append(self.field[y][x - 1])

        if y - 1 >= self.min_coord:
            if self.is_living(self.field[y - 1][x]):
                if self.still_living.get(self.field[y - 1][x]).unit_type == Game.prey_code:
                    preys.append(self.field[y - 1][x])

        return preys

    def go_to(self, id_, coord):
        x, y = self.still_living[id_].coord
        self.field[y][x] = Game.empty_place_code
        self.still_living[id_].coord = coord
        x, y = coord
        self.field[y][x] = id_

    def remove(self, id_):
        if self.still_living[id_].unit_type == Game.predator_code:
            self.predators_amount -= 1

        if self.still_living[id_].unit_type == Game.prey_code:
            self.preys_amount -= 1

        x, y = self.still_living[id_].coord
        self.field[y][x] = Game.empty_place_code
        self.still_living.pop(id_)

    def full_turn(self):
        self.history.append((self.predators_amount, self.preys_amount))
        sentinel = 0
        self.queue.append(sentinel)
        while True:
            id_ = self.queue.popleft()
            if id_ == sentinel:
                break

            if self.is_living(id_):
                self.make_turn(id_)
                self.queue.append(id_)

        if self.predators_amount == 0 or self.preys_amount == 0:
            self.smb_is_living = False

    def is_living(self, id_):
        if self.still_living.get(id_) is None:
            return False

        return True

    def run(self, turns=0, output_every=0):
        current_turn = 1
        while True:
            if not self.smb_is_living:
                break
            self.full_turn()
            if current_turn == turns:
                break

            if output_every != 0:
                if current_turn % output_every == 0:
                    self.output()
                    print
            current_turn += 1

    def make_turn(self, id_):
        if self.still_living[id_].unit_type == Game.prey_code:
            self.make_prey_turn(id_)

        if self.still_living[id_].unit_type == Game.predator_code:
            self.make_predator_turn(id_)

    def make_predator_turn(self, id_):
        this = self.still_living[id_]
        possible_preys = self.preys_around(this.coord)
        if len(possible_preys) > 0 and this.last_feed > Unit.HUNGER_TRESHOLD:
            prey = choice(possible_preys)

            new_coord = self.still_living[prey].coord
            self.remove(prey)
            self.go_to(id_, new_coord)
            self.still_living[id_].feed()
            self.still_living[id_].last_birth += 1
        else:
            if (this.last_feed > Unit.MAX_HUNGER):
                self.remove(id_)
            else:
                self.still_living[id_].last_feed += 1
                self.make_prey_turn(id_)

    def make_prey_turn(self, id_):
        this = self.still_living[id_]
        possible_moves = self.empty_places_around(this.coord)
        if (Unit.PREGNANT_TIME <= this.last_birth and len(possible_moves) > 0):
            shift = choice(possible_moves)
            self.add_unit(this.unit_type, sum_pair(shift, this.coord))
            self.still_living[id_].after_birth()
        else:
            possible_moves.append(Game.same_place)
            shift = choice(possible_moves)
            self.go_to(id_, sum_pair(this.coord, shift))
            self.still_living[id_].last_birth += 1

if __name__ == "__main__":
    j = Game(3)
    j.add_unit(unit_type=Game.predator_code)
    j.add_unit(unit_type=Game.prey_code)

    j.output()

    j.run(output_every=1)
