#pragma once




#include "EditorPlugin.h"



namespace Urho3D
{

	class Window;
	class View3D;
	class Camera;
	class Node;
	class Scene;
	class CustomGeometry;
	class Texture2D;
	class Viewport;
	class EditorData;
	class EditorView;
	class EditorSelection;
	class DebugRenderer;
	class UI;
	class Input;
	class SoundListener;
	class Renderer;
	class ResourceCache;
	class Text;
	class Font;
	class LineEdit;
	class CheckBox;
	class XMLFile;
	class Menu;
	class FileSelector;
	class FileSystem;
	class File;
	class Editor;
	class Button;

	class EPScene2D;

	class EPScene2DEditor : public Object
	{
		OBJECT(EPScene2DEditor);
		friend class EPScene2D;
	public:
		/// Construct.
		EPScene2DEditor(Context* context);
		/// Destruct.
		virtual ~EPScene2DEditor();

		void Edit(Object *object);
		void Show();
		void Hide();

	protected:
		/// cache editor subsystems
		EditorData*			editorData_;
		EditorView*			editorView_;
		EditorSelection*	editorSelection_;
		Editor*				editor_;
		/// cache subsystems
		UI*				ui_;
		Input*			input_;
		Renderer*		renderer;
		ResourceCache*	cache_;
		FileSystem*		fileSystem_;


		SharedPtr<Window> window_;

	};

	class EPScene2D : public EditorPlugin
	{

	    OBJECT(EPScene2D);

	public:
		/// Construct.
		EPScene2D(Context* context);
		/// Destruct.
		virtual ~EPScene2D();

		virtual bool HasMainScreen() override;

		virtual String GetName() const override;

		virtual void Edit(Object *object) override;

		virtual bool Handles(Object *object) const override;

		virtual UIElement* GetMainScreen() override;

		virtual void SetVisible(bool visible) override;

	protected:
		SharedPtr<Window> window_;

	};
}
