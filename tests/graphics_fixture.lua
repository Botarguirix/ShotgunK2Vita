-- Self-authored visual regression scene; copied game fonts remain private.
function _init()
 newsrf(64,64,"fixture")
 target("fixture")
 for y=0,63 do for x=0,63 do pset(x,y,(x*7+y*11)%32) end end
 target()
end
function _update() end
function _draw()
 target();camera();clip();pal();palt();sfillp();blend();fillp();cls(3)
 rectfill(-300,-300,30,15,22)
 camera(7,-2);clip(8,7,290,153)
 spritesheet("fixture")
 sspr(2,5,60,54,-18,-10,350,189)
 sspr(0,0,32,32,80,30,-63,47)
 sspr(0,0,64,64,95,85,128,-80)
 pal(5,27);palt(17,true);sfillp(12,23130,28+29*256)
 sspr(-12,-8,80,80,112,59,97,71)
 fillp(23130,true);rectfill(40,35,120,80,20+25*256)
 fillp(43690,false);rectfill(220,10,450,600,10+13*256)
 fillp();blend(5,3,9);rectfill(-9,0,20,300,5)
 camera();clip();font("default");print("Cache outline 123",38,150,24)
 print("Cache outline 123",37,149,6)
end
