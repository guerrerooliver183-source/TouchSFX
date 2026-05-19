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
 * Se usa getSaveDir() para que el archivo persista correctamente.
 */
void checkAndDownloadSFX() {
    auto sfxPath = Mod::get()->getSaveDir() / SFX_FILE;
    
    // Añadimos el directorio de guardado a las rutas de búsqueda de Cocos2d
    // Esto es crucial para que FMOD pueda encontrar el archivo descargado
    CCFileUtils::get()->addSearchPath(Mod::get()->getSaveDir().string().c_str());

    if (!std::filesystem::exists(sfxPath)) {
        log::info("SFX no encontrado, iniciando descarga: {}", SFX_URL);
        
        web::AsyncWebRequest()
            .fetch(SFX_URL)
            .into(sfxPath)
            .then([sfxPath](auto) {
                log::info("SFX descargado exitosamente en: {}", sfxPath.string());
                // Forzamos la actualización de las rutas de búsqueda tras la descarga
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
 * Se ejecuta cuando el mod se carga.
 */
$on_mod(Loaded) {
    checkAndDownloadSFX();
}

/**
 * Hookeamos CCLayer para detectar toques globales.
 */
class $modify(MyCCLayer, CCLayer) {
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        // Llamamos a la función original
        bool result = CCLayer::ccTouchBegan(touch, event);

        // Verificamos si el mod está habilitado
        if (Mod::get()->getSettingValue<bool>("enabled")) {
            // Reproducimos el sonido
            // FMODAudioEngine maneja automáticamente si el archivo está en el SearchPath
            FMODAudioEngine::sharedEngine()->playEffect(SFX_FILE);
            
            log::debug("Touch detectado, reproduciendo SFX");
        }

        return result;
    }
};
