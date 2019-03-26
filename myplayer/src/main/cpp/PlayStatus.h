//
// Created by libowen on 19-3-20.
//

#ifndef MYMUSIC_PLAYSTATUS_H
#define MYMUSIC_PLAYSTATUS_H


class PlayStatus {

public:
    bool exit;
    bool load;
    bool seek = false;

public:
    PlayStatus();
};


#endif //MYMUSIC_PLAYSTATUS_H
