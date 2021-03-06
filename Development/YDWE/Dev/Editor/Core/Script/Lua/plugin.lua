
plugin = {}
plugin.loaders = {}
plugin.path = fs.ydwe_path() / "plugin"

function plugin.load (self, plugin_config_path)
	log.trace("Load plugin config " .. plugin_config_path:string())
	
	local plugin_config = sys.config_property(plugin_config_path)
	local plugin_name = plugin_config:get_string('Info.PluginName', '')
	
	if plugin_name == '' then
		log.error("Cannot plugin name.")
		return
	end

	if plugin_name == 'YDTileLimitBreaker' or plugin_name == 'YDCustomObjectId' then
		log.trace("Blacklist.")
		return
	end
	
	if self.loaders[plugin_name] then
		log.error(plugin_name .. " already exists.")
		return
	end
	
	if 0 == plugin_config:get_integer('Load.Enable', 0) then
		log.debug("Diable " .. plugin_name .. ".")
		return		
	end
	
	local plugin_loader_path = plugin_config:get_string('Load.Loader', '')
	if plugin_loader_path == '' then
		log.error("Cannot find " .. plugin_name .. "'s loader.")
		return 
	end
	plugin_loader_path = plugin_config_path:parent_path() / plugin_loader_path
	if not fs.exists(plugin_loader_path) then
		log.error(plugin_name .. "'loader does not exist.")
		return
	end
	
	local s, r = pcall(dofile, plugin_loader_path:string())
	if not s then
		log.error("Error in initialize " .. plugin_name .. "'s loader: ".. r)
		return 
	end
	
	self.loaders[plugin_name] = r

	local plugin_dll_path = plugin_config:get_string('Load.Dll', '')
	if plugin_dll_path == '' then
		log.error("Cannot find " .. plugin_name .. "'s dll.")
		return 
	end
	plugin_dll_path = plugin_config_path:parent_path() / plugin_dll_path
	if not fs.exists(plugin_dll_path) then
		log.error(plugin_name .. "'dll does not exist. " .. plugin_dll_path:string())
		return
	end
	
	s, r, msg = pcall(self.loaders[plugin_name].load, plugin_dll_path)
		
	if not s then
		log.error("Error in load " .. plugin_name .. ": ".. r)
		self.loaders[plugin_name] = nil
		return 
	end
	
	if not r then
		if msg then
			log.error(plugin_name .. " loading failed: " .. msg ..".")
		else
			log.error(plugin_name .. " loading failed.")
		end
		self.loaders[plugin_name] = nil
		return
	end
	
	log.debug(plugin_name .. " loaded successfully.")	
	return
end

function plugin.load_directory (self, plugin_dir)
	-- ����Ŀ¼
	for full_path in plugin_dir:list_directory() do
		if fs.is_directory(full_path) then
			self:load_directory(full_path)
		elseif full_path:extension():string() == ".plcfg" then
			self:load(full_path)
		end
	end
end

function plugin.load_all (self)
	self:load_directory(self.path)
end

function plugin.unload_all (self)
	for name, loader in pairs(self.loaders) do
		log.trace("Unload plugin " .. name .. ".")
		pcall(loader.unload)
	end
end
