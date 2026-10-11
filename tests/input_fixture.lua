function _init()
 controls("shoot>c:R2\npage>c:R1\nleft>c:lstick:left\ninfo>c:touchpad")
 n=0
end
function _update()
 n=n+1
 if n==1 then assert(not btn("shoot")) end
 if n==2 then assert(btn("shoot") and btnp("shoot") and not btn("page")) end
 if n==3 then assert(btn("shoot") and not btnp("shoot")) end
 if n==4 then assert(btnr("shoot") and btnp("page")) end
 if n==5 then assert(btnp("left") and not btn("info")) end
 if n==6 then assert(btn("left") and not btnp("left")) end
 if n==7 then assert(btnr("left")) end
end
function _draw() end
