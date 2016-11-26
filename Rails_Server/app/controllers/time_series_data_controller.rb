class TimeSeriesDataController < ApplicationController
  before_action :set_time_series_datum, only: [:show, :edit, :update, :destroy]

  # GET /time_series_data
  # GET /time_series_data.json
  def index
    @time_series_data = TimeSeriesDatum.all
  end

  # GET /time_series_data/1
  # GET /time_series_data/1.json
  def show
  end

  # GET /time_series_data/new
  def new
    @time_series_datum = TimeSeriesDatum.new
  end

  # GET /time_series_data/1/edit
  def edit
  end

  # POST /time_series_data
  # POST /time_series_data.json
  def create
    @time_series_datum = TimeSeriesDatum.new(time_series_datum_params)

    respond_to do |format|
      if @time_series_datum.save
        format.html { redirect_to @time_series_datum, notice: 'Time series datum was successfully created.' }
        format.json { render :show, status: :created, location: @time_series_datum }
      else
        format.html { render :new }
        format.json { render json: @time_series_datum.errors, status: :unprocessable_entity }
      end
    end
  end

  # PATCH/PUT /time_series_data/1
  # PATCH/PUT /time_series_data/1.json
  def update
    respond_to do |format|
      if @time_series_datum.update(time_series_datum_params)
        format.html { redirect_to @time_series_datum, notice: 'Time series datum was successfully updated.' }
        format.json { render :show, status: :ok, location: @time_series_datum }
      else
        format.html { render :edit }
        format.json { render json: @time_series_datum.errors, status: :unprocessable_entity }
      end
    end
  end

  # DELETE /time_series_data/1
  # DELETE /time_series_data/1.json
  def destroy
    @time_series_datum.destroy
    respond_to do |format|
      format.html { redirect_to time_series_data_url, notice: 'Time series datum was successfully destroyed.' }
      format.json { head :no_content }
    end
  end
  
  def upload
    if params[:csv_line]
      # 2015-06-01 13:59:59,"Northwest Corn Thingy",4.54,4.30,4.11,4.78
      fields = params[:csv_line].split(',')
      timestamp = Time.strptime(fields[0],"%Y-%m-%d %H:%M:%S")
      station = Station.find_by_name(fields[1].gsub('"',''))
      lc1 = fields[2].to_f
      lc2 = fields[3].to_f
      lc3 = fields[4].to_f
      lc4 = fields[5].to_f
      TimeSeriesDatum.create! datetime:timestamp, station:station, lc1:lc1,
        lc2:lc2, lc3:lc3, lc4:lc4
    end
    ts = Time.now.gmtime.to_i
    render text:"OK::#{ts}"
  end

  private
    # Use callbacks to share common setup or constraints between actions.
    def set_time_series_datum
      @time_series_datum = TimeSeriesDatum.find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def time_series_datum_params
      params.require(:time_series_datum).permit(:datetime, :station_id, :lc1, :lc2, :lc3, :lc4)
    end
end
