world.counter = 0

function actions_for_node(path)
    print("path: "..path)
    return {"fuck", "marry", "kill"}
end

function action_taken(action, target)
    print("action ("..action..") taken on: "..target)
    world.counter = world.counter + 1
    prompt_text("siemka", "balls")
end

ret = prompt_choice("siemka2", "test choice", {"a", "b", "c"});
print(ret)
world.counter = world.counter + 1
