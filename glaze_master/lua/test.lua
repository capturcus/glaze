function async_test(t)
    x = prompt_choice(t.player, "what do you choose", {"cat", "dog"})
    log("player "..t.player.." chose "..x)
    world.animal=x
end

run_in_background(async_test, {{player="siemka"}, {player="siemka2"}})
