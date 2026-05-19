#include <Geode/Geode.hpp>
#include <Geode/modify/CCLayer.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

// Nombre del archivo SFX
const std::string SFX_FILE = "s5939.ogg";
// URL de descarga de los servidores de RobTop
const std::string SFX_URL = "https://www.boomlings.com/database/sfx/" + SFX_FILE;

/**
 * Función para descargar el SFX si no existe.
 * En Geode v5, se recomienda el uso de getSaveDir para persistencia.
 */
void checkAndDownloadSFX() {
    auto sfxPath = Mod::get()->getSaveDir() / SFX_FILE;
    
    // Añadimos el directorio de guardado a las rutas de búsqueda de Cocos2d
    CCFileUtils::get()->addSearchPath(Mod::get()->getSaveDir().string().c_str());

    if (!std::filesystem::exists(sfxPath)) {
        log::info("SFX no encontrado, iniciando descarga: {}", SFX_URL);
        
        web::AsyncWebRequest()
            .fetch(SFX_URL)
            .into(sfxPath)
            .then([sfxPath](auto) {
                log::info("SFX descargado exitosamente en: {}", sfxPath.string());
                CCFileUtils::get()->addSearchPath(Mod::get()->getSaveDir().string().c_str());
            })
            .expect([](std::string const& error) {
                log::error("Error al descargar SFX: {}", error);
            });
    } else {
        log::info("SFX ya existe en: {}", sfxPath.string());
    }
}

/**
 * En Geode v5, $on_mod(Loaded) sigue siendo válido para inicialización.
 */
$on_mod(Loaded) {
    checkAndDownloadSFX();
}

/**
 * Hook de CCLayer para detectar toques globales.
 * Compatible con la estructura de Geode v5.
 */
class $modify(MyCCLayer, CCLayer) {
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        // Llamada a la función original
        bool result = CCLayer::ccTouchBegan(touch, event);

        // Acceso a settings actualizado para Geode v5
        if (Mod::get()->getSettingValue<bool>("enabled")) {
            // Reproducción de sonido con FMOD
            FMODAudioEngine::sharedEngine()->playEffect(SFX_FILE);
            
            log::debug("Touch detectado en Geode v5, reproduciendo SFX");
        }

        return result;
    }
};
