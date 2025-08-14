/**
 * QtAwesome - use font-awesome (or other font icons) in your c++ / Qt Application
 *
 * MIT Licensed
 *
 * Copyright 2013-2024 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "QtAwesome.h"
#include "QtAwesomeAnim.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPalette>
#include <QString>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#define USE_COLOR_SCHEME
#include <QStyleHints>
#include <QSvgRenderer>
#endif
#ifdef Q_OS_MAC
#include "osxstyle.h"
#endif

// Initializing namespaces need to happen outside a namespace
[[maybe_unused]] static void qtawesome_init_resources()
{
#ifdef BUNDLED_FONTAWESOME
	Q_INIT_RESOURCE(QtAwesomeFree);
#endif
}

namespace fa {

#include "QtAwesomeStringGenerated.h"

QtAwesomeIconPainter::~QtAwesomeIconPainter()
{
}

/// The font-awesome icon painter
class QtAwesomeCharIconPainter : public QtAwesomeIconPainter {
protected:
	virtual ~QtAwesomeCharIconPainter()
	{
	}

	QStringList optionKeysForModeAndState(const QString& key, QIcon::Mode mode, QIcon::State state)
	{
		QString modePostfix;
		switch (mode) {
		case QIcon::Disabled:
			modePostfix = "-disabled";
			break;
		case QIcon::Active:
			modePostfix = "-active";
			break;
		case QIcon::Selected:
			modePostfix = "-selected";
			break;
		default:
			break;
		}

		QString statePostfix;
		if (state == QIcon::Off) {
			statePostfix = "-off";
		}

		// the keys that need to bet tested:   key-mode-state | key-mode | key-state | key
		QStringList result;
		if (!modePostfix.isEmpty()) {
			if (!statePostfix.isEmpty()) {
				result.push_back(key + modePostfix + statePostfix);
			}
			result.push_back(key + modePostfix);
		}
		if (!statePostfix.isEmpty()) {
			result.push_back(key + statePostfix);
		}
		return result;
	}

	QVariant optionValueForModeAndState(const QString& baseKey, QIcon::Mode mode, QIcon::State state,
	                                    const QVariantMap& options, const QtAwesome* awesome)
	{
		for (const QString& key : optionKeysForModeAndState(baseKey, mode, state)) {
			if (options.contains(key) && !(options.value(key).toString().isEmpty())) {
				return options.value(key);
			}
		}

		if (options.contains(baseKey) && !(options.value(baseKey).toString().isEmpty())) {
			return options.value(baseKey);
		}

		for (const QString& key : optionKeysForModeAndState(baseKey, mode, state)) {
			if (awesome->hasDefaultOption(key) && !(awesome->defaultOption(key).toString().isEmpty())) {
				return awesome->defaultOption(key);
			}
		}

		return awesome->defaultOption(baseKey);
	}

public:
	void paint(QtAwesome* awesome, QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state,
	           const QVariantMap& options) override
	{
		painter->save();

		painter->setRenderHint(QPainter::Antialiasing);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
		painter->setRenderHint(QPainter::HighQualityAntialiasing);
#endif

		QVariant var = options.value("anim");
		QtAwesomeAnimation* anim = var.value<QtAwesomeAnimation*>();
		if (anim) {
			anim->setup(*painter, rect);
		}

		// set the default options
		QColor color = optionValueForModeAndState("color", mode, state, options, awesome).value<QColor>();
		QString text = optionValueForModeAndState("text", mode, state, options, awesome).toString();
		QString filename = optionValueForModeAndState("filename", mode, state, options, awesome).toString();
		int st = optionValueForModeAndState("style", mode, state, options, awesome).toInt();
		double scaleFactor = optionValueForModeAndState("scale-factor", mode, state, options, awesome).toDouble();

		bool rtl = optionValueForModeAndState("rtl", mode, state, options, awesome).toBool();

		Q_ASSERT(color.isValid());
		Q_ASSERT(!text.isEmpty() || !filename.isEmpty());

		// Setting RTL makes the icon RTL-aware and will flip it if
		// needed.
		if (rtl && QApplication::isRightToLeft()) {
			painter->translate(rect.width(), 0);
			painter->scale(-1, 1);
		}

		if (filename.isEmpty()) {
			painter->setPen(color);
			QRectF textRect(rect);
			int flags = Qt::AlignHCenter | Qt::AlignVCenter;

			// ajust font size depending on the rectangle
			int drawSize = qRound(textRect.height() * scaleFactor);
			QFont ft = awesome->font(st, drawSize);
			QFontMetricsF fm(ft);
			QRectF tbr = fm.boundingRect(textRect, flags, text);
			if (tbr.width() > textRect.width()) {
				drawSize = static_cast<int>(ft.pixelSize() * qMin(textRect.width() * 0.95 / tbr.width(), textRect.height() * 0.95 / tbr.height()));
				ft.setPixelSize(drawSize);
			}

			painter->setFont(ft);
			painter->drawText(textRect, flags, text);
		}
		else {
			QSvgRenderer renderer;
			QFile f(filename);
			QByteArray bytes;
			if (f.open(QIODevice::ReadOnly)) {
				bytes = f.readAll();
			}
			if (!bytes.isEmpty()) {
				bytes.replace("#000", color.name().toLatin1());
			}
			renderer.load(bytes);
			renderer.render(painter, QRect(0, 0, rect.width(), rect.height()));
		}

		painter->restore();
	}
};

//---------------------------------------------------------------------------------------

/// The painter icon engine.
class QtAwesomeIconPainterIconEngine : public QIconEngine {

public:
	QtAwesomeIconPainterIconEngine(QtAwesome* awesome, QtAwesomeIconPainter* painter, const QVariantMap& options)
		: awesomeRef_(awesome), iconPainterRef_(painter), options_(options)
	{
	}

	virtual ~QtAwesomeIconPainterIconEngine() {}

	QtAwesomeIconPainterIconEngine* clone() const
	{
		return new QtAwesomeIconPainterIconEngine(awesomeRef_, iconPainterRef_, options_);
	}

	virtual void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
	{
		Q_UNUSED(mode);
		Q_UNUSED(state);
		iconPainterRef_->paint(awesomeRef_, painter, rect, mode, state, options_);
	}

	virtual QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
	{
		QPixmap pm(size);
		pm.fill(Qt::transparent);// we need transparency
		{
			QPainter p(&pm);
			paint(&p, QRect(QPoint(0, 0), size), mode, state);
		}
		return pm;
	}

private:
	QtAwesome* awesomeRef_;               ///< a reference to the QtAwesome instance
	QtAwesomeIconPainter* iconPainterRef_;///< a reference to the icon painter
	QVariantMap options_;                 ///< the options for this icon painter
};

//---------------------------------------------------------------------------------------

const QString QtAwesome::FA_BRANDS_FONT_FILENAME = "Font Awesome 6 Brands-Regular-400.otf";
const QString QtAwesome::FA_REGULAR_FONT_FILENAME = "Font Awesome 6 Free-Regular-400.otf";
const QString QtAwesome::FA_SOLID_FONT_FILENAME = "Font Awesome 6 Free-Solid-900.otf";
const QString QtAwesome::FA_BRANDS_FONT_STYLE = "Regular";
const QString QtAwesome::FA_REGULAR_FONT_STYLE = "Regular";
const QString QtAwesome::FA_SOLID_FONT_STYLE = "Solid";

/// The default icon colors
QtAwesome::QtAwesome(QObject* parent)
	: QObject(parent), _namedCodepointsByStyle(), _namedCodepointsList()
{
	hasInit = false;

	resetDefaultOptions();

	_fontIconPainter = new QtAwesomeCharIconPainter();

#ifdef BUNDLED_FONTAWESOME
	_fontDetails.insert(fa::fa_solid, QtAwesomeFontData(FA_SOLID_FONT_FILENAME, FA_SOLID_FONT_WEIGHT, FA_SOLID_FONT_STYLE));
	_fontDetails.insert(fa::fa_regular, QtAwesomeFontData(FA_REGULAR_FONT_FILENAME, FA_REGULAR_FONT_WEIGHT, FA_REGULAR_FONT_STYLE));
	_fontDetails.insert(fa::fa_brands, QtAwesomeFontData(FA_BRANDS_FONT_FILENAME, FA_BRANDS_FONT_WEIGHT, FA_BRANDS_FONT_STYLE));
#else
	// use "filename" to store font family name...
	_fontDetails.insert(fa::fa_solid, QtAwesomeFontData("Font Awesome 6 Free Solid", FA_SOLID_FONT_WEIGHT, FA_SOLID_FONT_STYLE));
	_fontDetails.insert(fa::fa_regular, QtAwesomeFontData("Font Awesome 6 Free Regular", FA_REGULAR_FONT_WEIGHT, FA_REGULAR_FONT_STYLE));
	_fontDetails.insert(fa::fa_brands, QtAwesomeFontData("Font Awesome 6 Brands Regular", FA_BRANDS_FONT_WEIGHT, FA_BRANDS_FONT_STYLE));
#endif

#ifdef USE_COLOR_SCHEME
	// support dark/light mode
	QObject::connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme _) {
		resetDefaultOptions();
	});
#endif
}

void QtAwesome::resetDefaultOptions()
{
	_defaultOptions.clear();

	setDefaultOption("color", QApplication::palette().color(QPalette::Normal, QPalette::Text));
	setDefaultOption("color-disabled", QApplication::palette().color(QPalette::Disabled, QPalette::Text));
	setDefaultOption("color-active", QApplication::palette().color(QPalette::Active, QPalette::Text));
#ifdef Q_OS_MAC
	setDefaultOption("color-selected", OSXStyle::self()->viewPalette().highlightedText().color());
#else
	setDefaultOption("color-selected", QApplication::palette().color(QPalette::Active, QPalette::HighlightedText));// TODO: check how to get the correct highlighted color
#endif
	setDefaultOption("scale-factor", 1.0);

	setDefaultOption("text", QVariant());
	setDefaultOption("text-disabled", QVariant());
	setDefaultOption("text-active", QVariant());
	setDefaultOption("text-selected", QVariant());

	Q_EMIT defaultOptionsReset();
}

QtAwesome::~QtAwesome()
{
	delete _fontIconPainter;
	qDeleteAll(_painterMap);
	qDeleteAll(_namedCodepointsList);
}

/// a specialized init function so font-awesome is loaded and initialized
/// this method return true on success, it will return false if the fnot cannot be initialized
/// To initialize QtAwesome with font-awesome you need to call this method
bool QtAwesome::initFontAwesome()
{
	if (hasInit) return true;
	bool success = true;

#ifdef BUNDLED_FONTAWESOME
	// The macro below internally calls "qInitResources_QtAwesome()". this initializes
	// the resource system. For a .pri project this isn't required, but when building and using a
	// static library the resource need to initialized first.
	///
	// I've checked th qInitResource_* code and calling this method mutliple times shouldn't be any problem
	// (More info about this subject:  http://qt-project.org/wiki/QtResources)
	qtawesome_init_resources();

	for (QtAwesomeFontData& fd : _fontDetails) {
		// only load font-awesome once
		if (fd.fontId() < 0) {
			// load the font file
			QFile res(":/fonts/" + fd.fontFilename());
			if (!res.open(QIODevice::ReadOnly)) {
				qDebug() << "Font awesome font" << fd.fontFilename() << "could not be loaded!";
				success = false;
				continue;
			}
			QByteArray fontData(res.readAll());
			res.close();

			// fetch the given font
			fd.setFontId(QFontDatabase::addApplicationFontFromData(fontData));
		}

		QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(fd.fontId());
		if (loadedFontFamilies.empty()) {
			qDebug() << "Font awesome" << fd.fontFilename() << " font is empty?!";
			fd.setFontId(-1);// restore the font-awesome id
			return false;
		}
		else {
			fd.setFontFamily(loadedFontFamilies.at(0));
		}
	}
#else
	for (QtAwesomeFontData& fd : _fontDetails) {
		fd.setFontFamily(fd.fontFilename());// yes, what a great semantic use of the variable...
	}
#endif

	// intialize the brands icon map
	addToNamedCodePoints(fa::fa_brands, faBrandsIconArray, sizeof(faBrandsIconArray) / sizeof(QtAwesomeNamedIcon));
	addToNamedCodePoints(fa::fa_solid, faCommonIconArray, sizeof(faCommonIconArray) / sizeof(QtAwesomeNamedIcon));

	//initialize others code icons maps
	addToNamedCodePoints(fa::fa_regular, faRegularFreeIconArray, sizeof(faRegularFreeIconArray) / sizeof(QtAwesomeNamedIcon));

	hasInit = success;
	return success;
}

/// Add the given array as named codepoints
void QtAwesome::addToNamedCodePoints(int style, const QtAwesomeNamedIcon* QtAwesomeNamedIcons, int size)
{
	QHash<QString, int>* namedCodepoints = _namedCodepointsByStyle.value(style, nullptr);
	if (namedCodepoints == nullptr) {
		namedCodepoints = new QHash<QString, int>();
		_namedCodepointsList.append(namedCodepoints);
		_namedCodepointsByStyle.insert(style, namedCodepoints);
	}

	for (int i = 0; i < size; ++i) {
		namedCodepoints->insert(QtAwesomeNamedIcons[i].name, QtAwesomeNamedIcons[i].icon);
	}
}

const QHash<QString, int> QtAwesome::namedCodePoints(int style) const
{
	if (!_namedCodepointsByStyle.contains(style)) return QHash<QString, int>();
	return *_namedCodepointsByStyle[style];
}

/// Sets a default option. These options are passed on to the icon painters
void QtAwesome::setDefaultOption(const QString& name, const QVariant& value)
{
	_defaultOptions.insert(name, value);
}

/// Returns the default option for the given name
QVariant QtAwesome::defaultOption(const QString& name) const
{
	return _defaultOptions.value(name);
}

bool QtAwesome::hasDefaultOption(const QString& name) const
{
	return _defaultOptions.contains(name);
}

/// Creates an icon with the given code-point for given style
/// <code>
///     awesome->icon( icon_group )
/// </code>
QIcon QtAwesome::icon(int style, int character, const QVariantMap& options)
{
	auto optionMap = options;
	optionMap.insert("text", QString(QChar(character)));
	optionMap.insert("style", style);

	return icon(_fontIconPainter, optionMap);
}

/// Creates an icon with the given name
///
/// You can use the icon names as defined on https://fontawesome.com/cheatsheet/free and
/// https://fontawesome.com/cheatsheet/pro adding the style prefix, e.g. "fa-solid fa-address-book"
/// (The fa- prefix for the icon name is optional)
///
/// @param name the style and name of the icon.
/// @param options extra option to pass to the icon renderer
QIcon QtAwesome::icon(const QString& name, const QVariantMap& options)
{
	// Filename
	if (name.startsWith(":")) {
		auto optionMap = options;
		optionMap.insert("filename", name);

		return icon(_fontIconPainter, optionMap);
	}
	// split the string in a style and icon name (and skip the fa- prefix if given)
	int spaceIndex = name.indexOf(' ');
	int style = fa::fa_solid;
	QString iconName;

	if (spaceIndex > 0) {
		QString styleName = name.left(spaceIndex);
		style = stringToStyleEnum(styleName.startsWith("fa-") ? styleName.mid(3) : name);
		iconName = name.mid(spaceIndex + 1);
	}
	else {
		iconName = name;
	}

	if (iconName.startsWith("fa-")) {
		iconName = iconName.mid(3);
	}

	// when it's a named codepoint
	if (_namedCodepointsByStyle.contains(style) && _namedCodepointsByStyle[style]->contains(iconName)) {
		return icon(style, _namedCodepointsByStyle[style]->value(iconName), options);
	}

	auto optionMap = options;
	optionMap.insert("style", style);

	// this method first tries to retrieve the icon via the painter map
	QtAwesomeIconPainter* painter = _painterMap.value(name);
	if (!painter) return QIcon();

	return icon(painter, optionMap);
}

/// Create a dynamic icon by simlpy supplying a painter object
/// The ownership of the painter is NOT transfered.
/// @param painter a dynamic painter that is going to paint the icon
/// @param optionmap the options to pass to the painter
QIcon QtAwesome::icon(QtAwesomeIconPainter* painter, const QVariantMap& optionMap)
{
	// Warning, when you use memoryleak detection. You should turn it off for the next call
	// QIcon's placed in gui items are often cached and not deleted when my memory-leak detection checks for leaks.
	// I'm not sure if it's a Qt bug or something I do wrong
	QtAwesomeIconPainterIconEngine* engine = new QtAwesomeIconPainterIconEngine(this, painter, optionMap);
	return QIcon(engine);
}

/// Adds a named icon-painter to the QtAwesome icon map
/// As the name applies the ownership is passed over to QtAwesome
///
/// @param name the name of the icon including the style
/// @param painter the icon painter to add for this name
void QtAwesome::give(const QString& name, QtAwesomeIconPainter* painter)
{
	delete _painterMap.value(name);// delete the old one
	_painterMap.insert(name, painter);
}

/// \brief QtAwesome::font Creates/Gets the icon font with a given size in pixels. This can be usefull to use a label for displaying icons
///
/// \param style Font Awesome style such as fas, fal, fab, fad or far
/// \param size point size of the font
/// \return the corresponding font
/// Example:
///
///    QLabel* label = new QLabel(QChar( icon_group ));
///    label->setFont(awesome->font(style::fas, 16))
QFont QtAwesome::font(int style, int size) const
{
	if (!_fontDetails.contains(style)) return QFont();

	QFont font(_fontDetails[style].fontFamily());
	font.setPixelSize(size);
	font.setWeight(_fontDetails[style].fontWeight());
	font.setStyleName(_fontDetails[style].fontStyle());

	return font;
}

QString QtAwesome::fontName(int style) const
{
	if (!_fontDetails.contains(style)) return "";

	return _fontDetails[style].fontFamily();
}

int QtAwesome::stringToStyleEnum(const QString style) const
{
	if (style == "fa-solid") return fa::fa_solid;
	else if (style == "fa-regular")
		return fa::fa_regular;
	else if (style == "fa-brands")
		return fa::fa_brands;
	return fa::fa_solid;
}

const QString QtAwesome::styleEnumToString(int style) const
{
	switch (style) {
	case fa::fa_regular: return "fa-regular";
	case fa::fa_solid: return "fa-solid";
	case fa::fa_brands: return "fa-brands";
	}
	return "fa_solid";
}

//---------------------------------------------------------------------------------------

QtAwesomeFontData::QtAwesomeFontData(const QString& fontFileName, QFont::Weight fontWeight, const QString& fontStyle)
	: _fontFamily(QString()),
	  _fontFilename(fontFileName),
	  _fontId(-1),
	  _fontWeight(fontWeight),
	  _fontStyle(fontStyle)
{
}

const QString& QtAwesomeFontData::fontFamily() const
{
	return _fontFamily;
}

void QtAwesomeFontData::setFontFamily(const QString& family)
{
	_fontFamily = family;
}

const QString& QtAwesomeFontData::fontFilename() const
{
	return _fontFilename;
}

int QtAwesomeFontData::fontId() const
{
	return _fontId;
}

void QtAwesomeFontData::setFontId(int id)
{
	_fontId = id;
}

QFont::Weight QtAwesomeFontData::fontWeight() const
{
	return _fontWeight;
}

const QString& QtAwesomeFontData::fontStyle() const
{
	return _fontStyle;
}

///
/// \brief setFontWeight set the font weight as QFont::weight
/// \param weight the weight value according to QFont::weight enum
/// This enum contains the predefined font weights:
/// \value Thin       0         #same as weight 100
/// \value ExtraLight 12        #same as weight 200
/// \value Light      25        #same as weight 300
/// \value Normal     50        #same as weight 400
/// \value Medium     57        #same as weight 500
/// \value DemiBold   63        #same as weight 600
/// \value Bold       75        #same as weight 700
/// \value ExtraBold  81        #same as weight 800
/// \value Black      87        #same as weight 900
void QtAwesomeFontData::setFontWeight(QFont::Weight weight)
{
	_fontWeight = weight;
}

void QtAwesomeFontData::setFontStyle(const QString& style)
{
	_fontStyle = style;
}

}// namespace fa
