(import
 (owl toplevel)
 (prefix (owl sys) sys/))

(lambda (_)
  (print "Hello, World!")
  (print "(press RET to exit)")
  (sys/read stdin 1)
  0)
