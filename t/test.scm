(import
 (owl toplevel)
 (owl lazy)
 (owl sys)
 (prefix (owl sys) sys/)
 (lib))

(define (OK pred a b)
  (if (pred a b)
      (print "OK")
      (error "NOT OK" a)))

(λ (_)
  (OK eqv? (foo) 'bar)
  (OK eqv? (print "Hello, World!") #t)
  (OK eqv? (file? "Makefile") #t)

  0)
