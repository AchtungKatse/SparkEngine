#pragma once

SINLINE u64 string_hash(const char* string) {
    u64 hash = 5381;
    char c = *string;

    while (c != 0) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        c = *string++;
    }

    return hash;
}

SINLINE u64 hash_passthrough(u64 key) {
    return key;
}

// Stolen from stackoverflow. Performance may not be ideal
SINLINE u64 hash_u64(u64 key) {
    key = (key ^ (key >> 30)) * 0xbf58476d1ce4e5b9;
    key = (key ^ (key >> 27)) * 0x94d049bb133111eb;
    key = key ^ (key >> 31);
    return key;
}
