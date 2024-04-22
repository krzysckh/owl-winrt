(import
 (owl toplevel)
 (owl lazy)
 (owl sys)
 (lib))

(define (OK pred a b)
  (if (pred a b)
      (print "OK")
      (error "NOT OK" a)))

(define (test-networking)
  (print "testing networking...")
  (let ((con (open-connection (resolve-host "suckless.org") 80)))
    (write-bytes con (string->list "GET /\nHost: suckless.org\n\n"))
    (llen (lines con))))

(λ (_)
  (OK eqv? (foo) 'bar)
  (OK eqv? (print "Hello, World!") #t)
  (OK eqv? (file? "Makefile") #t)
  (OK > (test-networking) 0)

  0)
