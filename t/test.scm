(import
 (owl toplevel)
 (owl lazy)
 (prefix (owl sys) sys/)
 (lib))

(define (OK pred a b)
  (if (pred a b)
      (print "OK")
      (error "NOT OK" a)))

(λ (_)
  ;; (OK eqv? (foo) 'bar)
  ;; (OK eqv? (print "Hello, World!") #t)
  ;; (OK eqv? (file? "Makefile") #t)

  (print 'a)

  (define f (open-input-file "lsdfkjflsdjk"))
  (print (readable? f))
  (print "signgngs")

  (thread
   's0
   (let ((c (open-connection (sys/resolve-host "suckless.org") 80)))
     (sleep 100)
     (print-to c "GET / HTTP/1.0\r\n\r\n")
     (print (list->string (bytevector->list (try-get-block c 16 #t))))
     (close-port c)))

  (thread
   'waiter
   (let loop ()
     (print "got " (wait-mail))
     (loop)))

  (thread
   's1
   (let loop ((x 10))
     (print x)
     (if (< x 0)
         0
         (begin
           (sleep 200)
           (loop (- x 1))))))
  (sleep 250)
  (thread
   's2
   (let loop ((x 10))
     (print x)
     (when (= 0 (modulo x 2))
       (mail 'waiter `(helo ,x)))

     (if (< x 0)
         0
         (begin
           (sleep 200)
           (loop (- x 1))))))

  0)
