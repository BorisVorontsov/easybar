#ifndef SETTINGS_H
#define SETTINGS_H

#define EB_REG_GENERAL_PATH			L"Software\\EasyBar\\General"
#define EB_REG_FAVORITES_PATH		L"Software\\EasyBar\\Favorites"
#define EB_REG_PLAYBACK_PATH		L"Software\\EasyBar\\Playback"
#define EB_REG_DMO_PATH				L"Software\\EasyBar\\DMO"
#define EB_REG_EF_PATH				L"Software\\EasyBar\\External Filters"

//Опциональные ключи
#define EB_REG_RESTRICTIONS_PATH	L"Software\\EasyBar\\Restrictions"

void SetDefaultValues();
void GetSettings();
void SaveSettings();

#endif