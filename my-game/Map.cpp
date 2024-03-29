#include "Map.h"


LPTILESET getTileSetFromMin(unordered_map<int, LPTILESET>* tileset, int cell) {
	for (auto it = tileset->begin(); it != tileset->end(); ++it) {
		int id = it->first;
		if (cell > id) return it->second;
	}
	return tileset->begin()->second;
}


void Tileset::load(LPCWSTR path) {

}

void Tileset::unload() {
	texture = NULL;
		//delete ;
}

void Layer::render(unordered_map<int, LPTILESET>* tileset) {
	//DebugOut(L"[INFO] Render tile layer %s\n", ToLPCWSTR(this->name));
	for (int i = 0; i < width * height; i++) {
		int cell = data[i];
		if (cell > 0) {
			LPTILESET activeTileset = getTileSetFromMin(tileset, cell);

			Vector p;
			Rect r;

			double col = floor(i / width);
			double row = i % width;

			p.x = (float)row * activeTileset->tileHeight;
			p.y = (float)col * activeTileset->tileHeight; //calc from i, all size will be transform below

			double tilecol = floor(cell / activeTileset->imageWidth);
			double tileRow = cell % int(activeTileset->imageWidth);

			r.top = ((cell - activeTileset->firstGrid) / activeTileset->columns) * activeTileset->tileHeight;
			r.left = ((cell - activeTileset->firstGrid) % activeTileset->columns) * activeTileset->tileWidth;
			r.bottom = r.top + activeTileset->tileHeight;
			r.right = r.left + activeTileset->tileWidth;

			if (activeTileset != NULL && activeTileset->texture != NULL) {
				draw(p, activeTileset->texture, r, 255);
			}
		}
	}

}

void Layer::draw(Vector p, LPDIRECT3DTEXTURE9 texture, Rect r, int opacity) {
	// transform every size to be one fixed size
	// also calc new position after scale
	// draw here
	CGame* game = CGame::GetInstance();

	D3DXVECTOR2 pos, scale;

	float width = r.right - r.left;
	float height = r.bottom - r.top;
	// assum that width = height;

	scale.x = (FIXED_TILE_SIZE / width);
	scale.y = (FIXED_TILE_SIZE / height);

	scale.x = 1;
	scale.y = 1;



	game->DrawWithScale(p, texture, r, 255, scale);

	//game->Draw(p.x, p.y, texture, r.left, r.top, r.right, r.bottom, 255);
}

void Layer::unload() {

}

void Map::load(string path, vector<LPGAMEOBJECT>* obCollisions, LPSCENE scene) {
	DebugOut(L"Load file map \n");
	json mapData = ReadJsonFIle(ToLPCWSTR(path));
	this->data = mapData;
	DebugOut(L"Load done file map \n");


	//load all tileset first

	json tilesetData = mapData["tilesets"];
	for (json::iterator set = tilesetData.begin(); set != tilesetData.end(); ++set) {
		json data = set.value();
		Tileset* tile = new Tileset();
		int firstGrid = data["firstgid"];
		tile->firstGrid = data["firstgid"];
		tile->image = data["image"];
		tile->tileHeight = data["tileheight"];
		tile->tileWidth = data["tilewidth"];
		tile->imageHeight = data["imageheight"];
		tile->imageWidth = data["imagewidth"];
		tile->margin = data["margin"];
		tile->spacing = data["spacing"];
		tile->name = data["name"];
		tile->columns = data["columns"];


		string directory;
		const size_t last_slash_idx = path.rfind('/');
		if (std::string::npos != last_slash_idx)
		{
			directory = path.substr(0, last_slash_idx);
		}

		LPCWSTR filePath = ToLPCWSTR(directory + "/" + tile->image);


		tile->texture = CGame::LoadTexture(filePath);

		this->all_typeset[firstGrid] = tile;

	}
	DebugOut(L"Load tile set done\n");


	//load all layer

	json layerData = mapData["layers"];
	for (json::iterator set = layerData.begin(); set != layerData.end(); ++set) {
		json data = set.value();
		string type = string(data["type"]);
		string name = string(data["name"]);
		Layer* layer = new Layer();
		if (type == "tilelayer") {
			int id = data["id"];
			int height = data["height"];
			int width = data["width"];
			layer->name = string(data["name"]);
			layer->opacity = data["opacity"];
			layer->type = TILE_LAYER;
			layer->x = float(data["x"]);
			layer->y = float(data["y"]);
			layer->width = width;
			layer->height = height;
			layer->data = new int[width * height];

			json grid = data["data"];
			int i = 0;

			for (json::iterator cell = grid.begin(); cell != grid.end(); ++cell) {
				int value = cell.value();
				layer->data[i] = value;
				i = i + 1;
			}
			this->all_layer[id] = layer;
		}
		else if (type == "objectgroup") {
			delete layer;
			scene->ParseMapObject(data, obCollisions);
		}
	}
	DebugOut(L"Load object done\n");

}

void Map::render() {
	for (auto layer : all_layer) {
		layer.second->render(&all_typeset);
	}
};


void Map::unload() {
	for (auto layer : all_layer) {
		layer.second->unload();
	}
	all_layer.clear();
	for (auto tileset : all_typeset) {
		tileset.second->unload();
	}
	all_typeset.clear();
}