#pragma once

#define TIME_GUARD

#define TG_START_YEAR 2012
#define TG_START_MONTH 8
#define TG_START_DAY 17

#define TG_END_YEAR 2012
#define TG_END_MONTH 12
#define TG_END_DAY 1

// проверка откручена ли дата, и в диапозоне ли текуща€€ дата, false - если не в диапазоне, тихий экзепшин если откручена дата
// по false рекомендуетс€ €вно портить рабочие данные программы
bool test_valid_time();

bool test_network_time();              

