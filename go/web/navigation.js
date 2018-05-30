
$(document).ready(function(){
	
	 $(function(){
	   
	  		t = $(".menu_"+location.pathname.substring(1)).children("a");
			$(t).parents("ul").parent("li").siblings(".active").removeClass();
			$(t).parents("ul").parent("li").attr("class","active");
			$(t).parent("li").siblings(".active").removeClass();
			$(t).parent("li").attr("class","active");
		});
	// === Sidebar navigation === //
	$('.submenu > a').click(function(e)
    {
		e.preventDefault();
		var submenu = $(this).siblings('ul');
		var li = $(this).parents('li');
		var submenus = $('#sidebar li.submenu ul');
		var submenus_parents = $('#sidebar li.submenu');
		var timesegg=$('#_timeseg');
		if(li.hasClass('open'))
		{
			if(($(window).width() > 768) || ($(window).width() < 479)) {
				submenu.slideUp();         
			} else {
				submenu.fadeOut(250);
			}
            li.removeClass('open');
		} 
		else 
		{
			if(($(window).width() > 768) || ($(window).width() < 479) ) {
				
				submenus.slideUp();			
				submenu.slideDown();
			} else {
				submenus.fadeOut(250);			
				submenu.fadeIn(250);
			}
			submenus_parents.removeClass('open');		
            li.addClass('open');
		}
	});

    
	
});

