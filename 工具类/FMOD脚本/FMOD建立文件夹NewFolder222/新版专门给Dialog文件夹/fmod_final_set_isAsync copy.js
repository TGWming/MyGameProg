studio.system.print("=== FMOD 2.02 批量设置isAsync（最终正确版）===");
studio.system.print("用户: TGWming");
studio.system.print("时间: 2025-08-06 03:26:43");
studio.system.print("目标: Dialog 文件夹内所有音频的module.isAsync设置");

var targetFolderName = "NewFolder222";
var processedCount = 0;
var checkedCount = 0;

function setIsAsyncForAudioFiles(folder, folderPath) {
    if (folder.items) {
        for (var i = 0; i < folder.items.length; i++) {
            var item = folder.items[i];
            
            if (item.isOfType && item.isOfType("EventFolder")) {
                setIsAsyncForAudioFiles(item, folderPath + "/" + item.name);
            } 
            else if (item.isOfType && item.isOfType("Event")) {
                var event = item;
                studio.system.print("处理事件: " + event.name);
                
                if (event.groupTracks && event.groupTracks.length > 0) {
                    for (var j = 0; j < event.groupTracks.length; j++) {
                        var groupTrack = event.groupTracks[j];
                        
                        if (groupTrack.modules && groupTrack.modules.length > 0) {
                            for (var k = 0; k < groupTrack.modules.length; k++) {
                                var module = groupTrack.modules[k];
                                checkedCount++;
                                
                                if (module.isOfType && module.isOfType("SingleSound") && module.audioFile) {
                                    studio.system.print("  处理SINGLE SOUND模块 " + checkedCount);
                                    
                                    // 检查当前isAsync状态
                                    if (module.hasOwnProperty('isAsync') || module.isAsync !== undefined) {
                                        var oldValue = module.isAsync;
                                        
                                        try {
                                            module.isAsync = true;
                                            var newValue = module.isAsync;
                                            
                                            studio.system.print("    ✓ 设置 module.isAsync: " + oldValue + " -> " + newValue);
                                            processedCount++;
                                            
                                        } catch (e) {
                                            studio.system.print("    × 设置失败: " + e.toString());
                                        }
                                    } else {
                                        studio.system.print("    × 模块没有isAsync属性");
                                    }
                                }
                            }
                        }
                    }
                } else {
                    studio.system.print("  × 事件没有GroupTracks");
                }
            }
        }
    }
}

function findTargetFolder(folder, targetName) {
    if (folder.name === targetName) {
        return folder;
    }
    
    if (folder.items) {
        for (var i = 0; i < folder.items.length; i++) {
            var item = folder.items[i];
            if (item.isOfType && item.isOfType("EventFolder")) {
                var result = findTargetFolder(item, targetName);
                if (result) return result;
            }
        }
    }
    return null;
}

try {
    var masterFolder = studio.project.workspace.masterEventFolder;
    var targetFolder = findTargetFolder(masterFolder, targetFolderName);
    
    if (targetFolder) {
        studio.system.print("找到目标文件夹: " + targetFolderName);
        setIsAsyncForAudioFiles(targetFolder, "event:/" + targetFolderName);
        
        studio.system.print("=== 批量设置完成 ===");
        studio.system.print("检查了 " + checkedCount + " 个音频模块");
        studio.system.print("成功设置 " + processedCount + " 个模块的isAsync属性");
        studio.system.print("操作完成时间: 2025-08-06 03:26:43");
        
        // 保存项目
        try {
            studio.project.save();
            studio.system.print("✓ 项目已保存");
        } catch (e) {
            studio.system.print("× 保存项目失败: " + e.toString());
        }
        
        studio.system.print("\n请检查FMOD Studio界面确认ASYNC已经开启！");
        
    } else {
        studio.system.print("错误: 未找到名为 '" + targetFolderName + "' 的文件夹");
    }
} catch (error) {
    studio.system.print("执行出错: " + error.toString());
}