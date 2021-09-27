function async_test(player)
    x = prompt_choice(player, "what do you choose", {"cat", "dog"})
    log("player "..player.." chose "..x)
    world.animal=x
end
