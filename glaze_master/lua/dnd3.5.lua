creature = {
    strength = 0,
    dexterity = 0,
    constitution = 0,
    wisdom = 0,
    intelligence = 0,
    charisma = 0
}

races = {"Human", "Halfling", "Gnome", "Half Orc", "Elf", "Half Elf", "Dwarf"}

classes = {
    "Barbarian", "Bard", "Cleric", "Druid", "Fighter", "Monk", "Paladin",
    "Ranger", "Rogue", "Sorcerer", "Wizard"
}

monsters = {"gnoll"}

function score_modifier(score) return math.floor(score / 2. - 5) end

function table_remove_by_value(t, value)
    local key = nil
    for k, v in pairs(t) do if v == value then key = k end end
    if key ~= nil then t[key] = nil end
end

function spawn_monster(monster) world[monster] = {hp = 5} end

function create_player(player)
    local player_name = player.name
    print("creating player", player_name)
    prompt_text(player_name,
                "Throw 4 d6 dice. Ignore the lowest roll and add the remaining 3 together. Input the results in the following dialogs.")
    local throws = {}
    for i = 1, 6, 1 do
        throws[i] = prompt_number_response(player_name,
                                           "The result of throw " .. tostring(i) ..
                                               ":")
    end
    local race = prompt_choice(player_name, "Choose your race:", races)
    local class = prompt_choice(player_name, "Choose your class:", classes)
    local character = {}
    for k, v in pairs(creature) do
        local c = prompt_choice(player_name, "Choose a number for " .. k .. ":", throws)
        character[k] = c
        table_remove_by_value(throws, c)
    end
    character.race = race
    character.class = class
    world[player_name] = character
end

function action_taken(player, action, target)
    print("action_taken", player, action, target)
    if player == "master" then
        if action == "spawn_monster" then
            local m = prompt_choice("master", "Which monster do you want to spawn?",
                              monsters)
            spawn_monster(m)
        elseif action == "create_players" then
            local players = get_players()
            local i = 1
            player_objs = {}
            while players[i] ~= nil do
                table.insert(player_objs, { name = players[i] })
                i = i+1
            end
            run_in_background(create_player, player_objs)
        end
    end
end

function actions_for_node(player, path)
    if path ~= "" then
        return {}
    elseif player == "master" then
        return {"spawn_monster", "create_players", "create_player"}
    else
        return {}
    end
end
