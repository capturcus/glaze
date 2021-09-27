creature = {
    strength = 0,
    dexterity = 0,
    constitution = 0,
    wisdom = 0,
    intelligence = 0,
    charisma = 0,
}

function score_modifier(score) 
    return math.floor(score/2. - 5)
end

