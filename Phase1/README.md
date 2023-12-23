## CN Project Phase 1

Author: B09901027

#### Part 1: Socket text communication

``Compile``

In ```src/``` directory, run ``make``. The ``server`` and ``client`` binary will be compiled under your directory. Run ``make clean`` to remove the binary.

``Server``
Run ``./server [port num]`` to start. The log will show connection logs, including client connection, message and status.

``Client``
Run ``./client [hostname] [port num]`` to start. The log will show connection logs and server messages. The client supports hostname translation (ex. ``localhost`` to ``127.0.0.1``.)

#### Part 2: Profile page

A simple profile page written in html (``profile.html``) is provided. Open it through your favorite browser to check it.