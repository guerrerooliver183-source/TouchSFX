#include <Geode/Geode.hpp>
#include <Geode/modify/CCLayer.hpp>
#include <Geode/utils/web.hpp>
#include <fstream>

using namespace geode::prelude;

// SFX file name
const std::string SFX_FILE = "s5939.ogg";
// Download URL from RobTop's servers
const std::string SFX_URL = "https://www.boomlings.com/database/sfx/" + SFX_FILE;

/**
 * Function to download the SFX if it doesn't exist.
 * Updated for Geode v5 using the new WebRequest API.
 * The 'listen' method is not available in WebFuture. 
 * We use 'expect' and 'map' (or similar future handling) to process the result.
 */
void checkAndDownloadSFX() {
    auto sfxPath = Mod::get()->getSaveDir() / SFX_FILE;
    
    // Add the save directory to Cocos2d search paths
    CCFileUtils::get()->addSearchPath(Mod::get()->getSaveDir().string().c_str());

    if (!std::filesystem::exists(sfxPath)) {
        log::info("SFX not found, starting download: {}", SFX_URL);
        
        // In Geode v5, WebRequest returns a WebFuture which is a specialized Future.
        // We use the 'map' or 'expect' pattern common in Geode's new async system.
        web::WebRequest()
            .get(SFX_URL)
            .send()
            .expect([sfxPath](std::string const& error) {
                log::error("Failed to download SFX: {}", error);
            })
            .map([sfxPath](web::WebResponse* response) {
                if (response && response->ok()) {
                    auto data = response->data();
                    std::ofstream file(sfxPath, std::ios::binary);
                    if (file.is_open()) {
                        file.write(reinterpret_cast<const char*>(data.data()), data.size());
                        file.close();
                        log::info("SFX successfully downloaded and saved at: {}", sfxPath.string());
                        // Refresh search paths
                        CCFileUtils::get()->addSearchPath(Mod::get()->getSaveDir().string().c_str());
                    } else {
                        log::error("Failed to open file for writing: {}", sfxPath.string());
                    }
                }
                return response;
            });
    } else {
        log::info("SFX already exists at: {}", sfxPath.string());
    }
}

/**
 * Runs when the mod is loaded.
 */
$on_mod(Loaded) {
    checkAndDownloadSFX();
}

/**
 * Hook CCLayer to detect global touches.
 * Compatible with Geode v5 structure.
 */
class $modify(MyCCLayer, CCLayer) {
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        // Call the original function
        bool result = CCLayer::ccTouchBegan(touch, event);

        // Check if the mod is enabled in settings
        if (Mod::get()->getSettingValue<bool>("enabled")) {
            // Play sound effect using FMOD
            FMODAudioEngine::sharedEngine()->playEffect(SFX_FILE);
            
            log::debug("Touch detected in Geode v5, playing SFX");
        }

        return result;
    }
};
