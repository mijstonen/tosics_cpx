#!/usr/local/bin/cpx
#+queue.hpp

    void
goo(channel::out<int> oqo)
{
    for (int i=0;oqo;++i) {
        int j=2*i;
        INFO("channel::out <<",VARVALS(i,j));
        oqo<< i<< j;
    }
}

    void
foo(channel::inp<int> iqi)
{
    while(iqi){
        int v,w;
        iqi>> v >> w;
        INFO("channel::inp >> ",VARVALS(v,w));
    }
};

#!
    INFO("ok");
    Queue<int> q(100);
    mutex m; // access cout mutal exclusive

#if 1

std::jthread t[] = {
    std::jthread([&]() {
        for (const auto& i : repeat(10)) {
            this_thread::yield();
            auto qpop=q.pop();
            {lock_guard lg(m); INFO("q->T1",VARVALS(qpop));}
        }
    }),
    std::jthread([&]() {
        for (const auto& i : repeat(10)) {
            this_thread::yield();
            auto qpop=q.pop();
            {lock_guard lg(m); INFO("q->T2",VARVALS(qpop));}
        }
    }),
    std::jthread([&]() {
        for (const auto& i : repeat(10)) {
            {lock_guard lg(m); INFO(VARVALS(i),"T3->q");}
            q.push(i);
            this_thread::yield();
        }
    }),
    std::jthread([&]() {
        for (const auto& i : repeat(10)) {
            {lock_guard lg(m); INFO(VARVALS(i),"T4->q");}
            q.push(i);
            this_thread::yield();
        }
    })
};

#elif 0
q.dump();
for (const auto& i : repeat(10)) {
    INFO(VARVALS(i));
    q.push(i);
    q.dump();
}
<?> foreach(repeat(2) as $i)<{>
INFO(HGREEN "\n********** <?=$i?> **********" NOCOLOR);
for (const auto& i : repeat(5)) {
    INFO(VARVALS(i,q.pop()));
    q.dump();
}
for (auto i : repeat(5)) {
    INFO(VARVALS(i));
    q.push(i);
    q.dump();
}
<}>
#@
q.dump();
#else
 goo(channel::out(q));
 foo(channel::inp(q));
#endif

